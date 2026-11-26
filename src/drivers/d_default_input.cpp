#include "d_default_input.h"
#include "../core/utils/logging.h"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ostream>
#include <poll.h>
#include <stdexcept>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <termios.h>
#include <unordered_map>
#include <utf8proc.h>
#include <utility>
#include <vector>

using namespace drivers;
using namespace d_default_input_internals;
using namespace std;
using namespace core;

enum class KEY_TYPES: uint8_t{
#define TYPE(name, value) name = value,
#include "../../data/key_types.def"
#undef TYPE
};

long long d_default_input_internals::make_key(int i, char c) {
    return (static_cast<long long>(i) << 8) | static_cast<unsigned char>(c);
}
static void dump_debug(string_view m_buff, State& state, Data& data, size_t index, string msg, string func){
    fstream file("logs/debug-dump", ios::app);
    file << "=============" << msg  << "=============" << endl;
    file << "BUFF: '" << m_buff << "'" << endl;
    file << "DATA: 'EVENT: Kind = " << to_string(static_cast<uint8_t>(data.event.kind));
    file << " modifers = " << to_string(static_cast<uint8_t>(data.event.modifer));
    file << " text = '" << data.event.text << "'";
    file << " Key = " << to_string(static_cast<uint8_t>(data.event.key));
    file << endl;
    file << "INDEX: " << index << endl;
    file << "STATE:  " << to_string(static_cast<uint8_t>(state)) << endl;
    file.flush();
    file.close();
}
char d_default_input_internals::parse_csi_data(std::string_view s, std::vector<int>& results){ 
    results.clear();
    int value = 0;
    bool has_diget = false;
    int i = 0;
    for (const char ch : s){
        if (0x30 <= ch && ch <= 0x39){ // is digit
            value = (value * 10) + (ch - '0');
            has_diget = true;
        }
        else if (ch == ';'){
            results.push_back(value); 
            value = 0;
            has_diget = true;
        }
        else if (0x40 <= ch && ch <= 0x7E){
            if (has_diget){
                results.push_back(value);
            }
            return ch;
        } else {
            break;
        }
        i ++;
    }
    return '\0';
}

size_t d_default_input_internals::ground(string_view m_buff, State& state, Data& data, Context& cont, size_t index){
    for (size_t i = index; i < m_buff.size(); i++){
        const char& ch = m_buff[i];
        if (ch >= '\x20' && ch <= '\x7e'){ // Check if character is in printable ASCII
            if (data.has_data()){
                data.push(); // Push since multiple ASCII in a row are separate events.
            } 
            if (data.event.modifer != modifers::none){
                data.event.kind = Kind::Key;
                data.event.key = static_cast<Key>(ch);
            } else {
                data.event.kind = Kind::Text;
                if (ch >= 'A' && ch <= 'Z'){
                    data.event.key = static_cast<Key>(ch + ('a' - 'A'));
                } else {
                    data.event.key = static_cast<Key>(ch);
                }
                data.event.text = string(1, ch);
            }
            continue;
        } 
        else if (ch == '\x1b'){ // Check if character is the escape character.
            if (data.has_data()){
                data.push(); // Push since this is the start of a new sequence.
            }
            state = State::Esc;
            return i;
        } 
        else if (cont.ctrl_map.find(ch) != cont.ctrl_map.end()){ // check if character is the control map
            if (data.has_data()){
                data.push(); // Push since a ctrl character is always independent.
            }
            pair<Key, int> key = cont.ctrl_map[ch];
            data.event.key = key.first;
            data.event.modifer = data.event.modifer | static_cast<modifers>(key.second);
            data.event.kind = Kind::Key;
            continue;
        }
        else if (ch >= '\xC2' && ch <= '\xF4'){ // Check if character is a Unicode first byte.
            state = State::Utf8;
            return i;
        }
        else { //Unknown data move on.
            if (data.has_data()){
                data.push(); // since this byte was unknown there is no chance later bytes can affect this event.
            }
            continue;
        }
    }
    return m_buff.size();
}
static size_t esc(string_view m_buff, State& state, Data& data, Context& cont,  size_t index){ // implement lone escape key detection.
    index += 1;
    if (index >= m_buff.size()){
        data.exit = true;
        return index - 1;
    }
    const char& ch = m_buff[index];
    switch (ch){
        case '[':
            state = State::Csi;
            break;
        case 'O':
            state = State::Ss3;
            break;
        case ']':
            state = State::String;
            break;
        case 'P':
            state = State::String;
            break;
        case '_':
            state = State::String;
            break;
        case '^':
            state = State::String;
            break;
        case 'X':
            state = State::String; 
            break;
        default:
            data.event.modifer = modifers::alt | data.event.modifer;
            state = State::Ground;
            return index;
    }
    return index;
}
size_t d_default_input_internals::paste(string_view m_buff, State& state, Data& data, Context& cont, size_t index){
    std::vector<int> results;
    bool was_esc = false;
    int csi_start = -1;
    for (size_t i = index; i < m_buff.size(); i++){
        const char& ch = m_buff[i];
        if (was_esc){
            if (ch == '['){
                csi_start = i + 1;
            }
            was_esc = false;
        }
        else if (csi_start != -1 && 0x40 <= ch && ch <= 0x7E){
            char last_byte = parse_csi_data(m_buff.substr(csi_start, (i - csi_start) + 1), results);
            if (results.size() == 1 && last_byte == '~' && results[0] == 201){
                data.event.kind = Kind::Paste;
                data.event.text = m_buff.substr(index, (csi_start - 2) - index); // subtract two to remove the \x1b[
                data.event.modifer = modifers::none;
                state = State::Ground;
                return i;
            }
        }
        else if (ch == '\x1b'){ // Check if character is the escape character.
            was_esc = true;
        } 
    }
    data.exit = true;
    return index;
}
size_t d_default_input_internals::ss3(string_view m_buff, State& state, Data& data, Context& cont, size_t index){ 
    index += 1;
    if (index >= m_buff.size()){
        data.exit = true;
        return index - 1;
    }
    const char& ch = m_buff[index];
    if (cont.ss3_map.find(ch) != cont.ss3_map.end()){
       data.event.kind = Kind::Key;
       data.event.key = cont.ss3_map[ch];
    } 
    else {
        logger::warn("SS3 End Byte '" + string(1, ch) + "' Is not in table.");
    }
    state = State::Ground;
    return index + 1;
}
size_t d_default_input_internals::csi(string_view m_buff, State& state, Data& data, Context& cont, size_t index){ 
    vector<int> results;
    for (size_t i = index + 1; i < m_buff.size(); i++){
        const char& ch  = m_buff[i];
        if (0x40 <= ch && ch <= 0x7E){
            char end_ch = parse_csi_data(m_buff.substr(index + 1, i - index), results);
            if (end_ch == '\0'){
                logger::error("Invalid csi data.");
                dump_debug(m_buff, state, data, index, "Invalid csi data.", "csi");
                return m_buff.size();
            }
            int param = 1;
            int modifer = 0;
            if (results.size() >= 1){
                param = results[0];
            } 
            if (end_ch == '~' && param == 200){
                state = State::Paste;
                return i + 1;
            }
            if (results.size() >= 2){
                modifer = results[1] - 1;
            }
            auto it = cont.csi_map.find(make_key(param, end_ch));
            if (it != cont.csi_map.end()){
                data.event.kind = Kind::Key;
                data.event.key = it->second;
                data.event.modifer = data.event.modifer | static_cast<modifers>(modifer);
            } else {
                logger::warn("CSI End Byte '" + string(1, end_ch) + "' With param '" + to_string(param) + "' Is not in table.");
            }
            state = State::Ground;
            return i + 1;
        }
    }
    data.exit = true;
    return index;
}
size_t d_default_input_internals::utf8(string_view m_buff, State& state, Data& data, Context& cont, size_t index){ // TODO make this function work better. Specifically zwj, as well as end of string cases.
    char first_byte = m_buff[index];
    int codepoint_len = InputString::get_byte_len(first_byte);
    if (codepoint_len > 4){
        dump_debug(m_buff, state, data, index, "Too long Unicode code point found.", "utf8");
        logger::warn("Too long Unicode code point found.");
        return m_buff.size();
    }
    char codepoint[5] {first_byte, '\0', '\0', '\0', '\0'};
    for (size_t i = index  + 1; i < m_buff.size(); i++){
        if (i - index  <= codepoint_len){
            codepoint[i - index] = m_buff[i];
        }
        if (i - index  >= codepoint_len - 1){
            utf8proc_int32_t cp;
            int consumed = utf8proc_iterate(reinterpret_cast<utf8proc_uint8_t*>(codepoint), codepoint_len, &cp);
            if (consumed < 0){ // invalid UTF-8
                dump_debug(m_buff, state, data, i, "Invalid utf8 bytes.", "utf8");
                logger::warn("Invalid utf8 bytes.");
                return m_buff.size();
            } 
            int catagory = utf8proc_category(cp);
            if (catagory == UTF8PROC_CATEGORY_MC || catagory == UTF8PROC_CATEGORY_ME || catagory == UTF8PROC_CATEGORY_MN){
                data.event.text += string(codepoint);
            } 
            else if (cp == 0x200D){
                data.event.text += string(codepoint);
            } 
            else {
                if (data.has_data()){
                    data.push();
                }
                data.event.kind = Kind::Text;
                data.event.text += string(codepoint);
            }
            state = State::Ground;
            return i + 1;
        }
    }
    data.exit = true;
    return index;
}
static size_t parseInput(string& buffer, State& s, Data& d, Context& c){
    size_t idx = 0;
    size_t prev_idx = 0;
    int no_change_ider = 0;
    int MAX_NO_IDX_CHANGE = 5;
    bool clean_state {false};

    while (idx < buffer.size()){
        if (d.exit){
            d.exit = false;
            break;
        }
        if (++no_change_ider >= MAX_NO_IDX_CHANGE){
            logger::error("Detected infinite loop.");
            dump_debug(buffer, s, d, idx, "", "parseInput");
            clean_state = true;
            break;
        }
        switch (s){
            case State::Ground:
                idx = ground(buffer, s, d, c, idx);
                break;
            case State::Utf8:
                idx = utf8(buffer, s, d, c, idx);
                break;
            case State::Esc:
                idx = esc(buffer, s, d, c, idx);
                break;
            case State::Ss3:
                idx = ss3(buffer, s, d, c, idx);
                break;
            case State::Csi:
                idx = csi(buffer, s, d, c, idx);
                break;
            case State::Paste:
                idx = paste(buffer, s, d, c, idx);
                break;
            default:
                logger::error("No function defined for state: " + to_string(static_cast<uint8_t>(s)));
                d.exit = true;
                clean_state = true;
        }
        if (idx > prev_idx){
            prev_idx = idx;
            no_change_ider = 0;
        }
    }
    if (d.has_data()){
        d.push();
    }
    if (clean_state){
        s = State::Ground;
        d.clear();
        return buffer.size();
    }
    return idx;
}
static int checkstdin(int timeout_ms){
    struct pollfd stdin_fd = {0, POLLIN, 0};
    int code = poll(&stdin_fd, 1, timeout_ms);
    if (code == -1){
        throw invalid_argument{"Error: poll raised error " + to_string(errno) + "."};
    }
    else if (code == 0){
        return 0;
    }
    if (!(stdin_fd.revents & POLLIN)){
        throw invalid_argument{"Error: poll raised error " + to_string(errno) + "."};
    } else {
        return 1;
    }
}
static int getDataSize(){
    int size;
    ioctl(0, FIONREAD, &size);
    return size;
}

static string getData(){
    int dataSize = 4096;
    if (dataSize){
        char buff[4096];
        int ammont = read(0, buff, dataSize);
        if (ammont == -1){
            return "";
        }
        return string(buff, ammont);
    }
    return "";
}
static string m_getCap(const char* name){
        char* s = tigetstr(name);
        if (s == (char*)-1 || s == nullptr) return {};
        return std::string(s);
}

void d_default_input::m_setRawMode() {
    tcgetattr(STDIN_FILENO, &m_old);
    termios t = m_old;
    t.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN );
    t.c_iflag &= ~(ICRNL | INLCR | IGNCR | IXON | ISTRIP | BRKINT | PARMRK | IGNBRK);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    // set to application cursor mode.
    int err = 0;
    if (setupterm(nullptr, STDOUT_FILENO, &err) != 0) {
        logger::critical("[Input] error in setupterm " + to_string(err));
        throw std::invalid_argument{"[Input] error in setupterm " + to_string(err)};
    }

    std::string smkx = m_getCap("smkx");
    std::string rmkx = m_getCap("rmkx");
    if (!smkx.empty()) putp(const_cast<char*>(smkx.c_str()));
}
void d_default_input::m_restoreMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &m_old);
    // reset from application cursor mode.
    const std::string rmkx = m_getCap("rmkx");
    if (!rmkx.empty()) putp(rmkx.c_str());
}
void d_default_input::m_fillDb(){
    if (m_cont.csi_map.empty() && m_cont.ss3_map.empty()){
#define KEY(name, id, type, param, last_byte)\
        if (type == static_cast<uint8_t>(KEY_TYPES::CSI)){\
            long long key = make_key(param, static_cast<char>(last_byte));\
            logger::debug("Adding CSI key: " + to_string(Key::name) + " param: " + to_string(param) + " last_byte: " + string(1, static_cast<char>(last_byte)) + " Key is: " + to_string(key));\
            m_cont.csi_map[key] = Key::name;\
        }\
        else if (type == static_cast<uint8_t>(KEY_TYPES::SS3)){\
            m_cont.ss3_map[static_cast<char>(last_byte)] = Key::name;\
            logger::debug("Adding SS3 key: " + to_string(Key::name) + " param: " + to_string(param) + " last_byte: " + string(1, static_cast<char>(last_byte)));\
        }\
        else if (type == static_cast<uint8_t>(KEY_TYPES::CTRL)){\
            logger::debug("Adding CTRL key: " + to_string(Key::name) + " param: " + to_string(param) + " last_byte: " + to_string(last_byte));\
            m_cont.ctrl_map[static_cast<char>(last_byte)] = pair<Key, int>(Key::name, param);\
        }
#include "../../data/keys.def"
#undef KEY
    }
}

vector<Event> d_default_input::d_getEvents(int timeout_ms){
    m_events.clear();
    if (!checkstdin(timeout_ms)){
        return m_events;
    }
    m_buff += getData();
    size_t idx = parseInput(m_buff, m_state, m_data, m_cont);
    m_buff.erase(m_buff.begin(), m_buff.begin() + idx);
    return m_events;
}
d_default_input::d_default_input(): m_data(m_events){
    m_setRawMode();
    m_fillDb();
}
d_default_input::~d_default_input(){
    m_restoreMode();
}
