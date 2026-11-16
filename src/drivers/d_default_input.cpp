#include "../core/drivers.h"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ostream>
#include <poll.h>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <unordered_map>
#include <utf8proc.h>
#include <utility>
#include <vector>

using namespace drivers;
using namespace std;
using namespace core;

enum class KEY_TYPES: uint8_t{
#define TYPE(name, value) name = value,
#include "../../data/key_types.def"
#undef TYPE
};
std::unordered_map<char, std::pair<Key, int>> ctrl_map {};
std::unordered_map<char, Key> ss3_map {};
std::unordered_map<char, Key> ckey_map {};

static long long make_key(int i, char c) {
    return (static_cast<long long>(i) << 8) | static_cast<unsigned char>(c);
}
namespace std{
    std::string to_string(Event e){
        return "Kind: " + to_string(static_cast<uint8_t>(e.kind)) +" Key: " + to_string(e.key) + " Text: " + e.text + " mods: " + to_string(static_cast<uint8_t>(e.modifer));
    }
}
std::unordered_map<long long, Key> csi_map {};

static void dump_debug(string& m_buff, State& state, Data& data, size_t index, string msg, string func){
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
static size_t ground(string& m_buff, State& state, Data& data, size_t index){
    for (size_t i = index; i < m_buff.size(); i++){
        char& ch = m_buff[i];
        if (ch >= '\x20' && ch <= '\x7e'){ // Check if character is in printable ASCII
            if (data.has_data()){
                data.push(); // Push since multiple ASCII in a row are separate events.
            } 
            data.event.kind = Kind::Text;
            data.event.text = string(1, ch);
            continue;
        } 
        else if (ch == '\x1b'){ // Check if character is the escape character.
            if (data.has_data()){
                data.push(); // Push since this is the start of a new sequence.
            }
            state = State::Esc;
            return i;
        } 
        else if (ctrl_map.find(ch) != ctrl_map.end()){ // check if character is the control map
            if (data.has_data()){
                data.push(); // Push since a ctrl character is always independent.
            }
            pair<Key, int> key = ctrl_map[ch];
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
static size_t esc(string& m_buff, State& state, Data& data, size_t index){ // implement lone escape key detection.
    index += 1;
    if (index >= m_buff.size()){
        data.exit = true;
        return index - 1;
    }
    char& ch = m_buff[index];
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
            data.event.kind = Kind::Key;
            data.event.key = static_cast<Key>(ch);
            data.event.modifer = modifers::alt | data.event.modifer;
            state = State::Ground;
            return index + 1;
    }
    return index;
}
static size_t ss3(string& m_buff, State& state, Data& data, size_t index){ 
    index += 1;
    if (index >= m_buff.size()){
        data.exit = true;
        return index - 1;
    }
    char& ch = m_buff[index];
    if (ss3_map.find(ch) != ss3_map.end()){
       data.event.kind = Kind::Key;
       data.event.key = ss3_map[ch];
       data.event.modifer = modifers::none;
    } 
    else {
        logger::warn("SS3 End Byte " + string(1, ch) + " Is not in table.");
    }
    state = State::Ground;
    return index + 1;
}
static size_t csi(string& m_buff, State& state, Data& data, size_t index){ 
    int digit_start = -1;
    int param = -1;
    for (size_t i = index + 1; i < m_buff.size(); i++){
        char& ch = m_buff[i];
        if (0x30 <= ch && ch <= 0x39){ // is digit
            if (digit_start == -1){
                digit_start = i;
            }
        }
        else if (ch == ';'){
            if (digit_start == -1){
                param = 0; // set number to 0 when there is no digit before the ; eg \x1b[;1A
            }
            else {
                param = stoi(m_buff.substr(digit_start, i - digit_start));
                digit_start = -1;
            }
        }
        else if (0x40 <= ch && ch <= 0x7E){
            if (param == -1){ // If the first parameter dose not exist then this is the first parameter.
                if (digit_start != -1){
                    param = stoi(m_buff.substr(digit_start, i - digit_start));
                    digit_start = -1;
                } else {
                    param = 1;
                }
            }
            long long key = make_key(param, ch);
            if (csi_map.find(key) != csi_map.end()){
                data.event.kind = Kind::Key;
                data.event.key = csi_map[key];
                if (digit_start != -1 && param != -1){ // If the first parameter already exist the second parameter should be treated as modifier.
                    data.event.modifer = static_cast<modifers>(stoi(m_buff.substr(digit_start, i - digit_start)) - 1);
                }
            } 
            state = State::Ground;
            return i + 1;
        }
    }
    data.exit = true;
    return index;
}
static size_t utf8(string& m_buff, State& state, Data& data, size_t index){ // TODO make this function work better. Specifically zwj, as well as end of string cases.
    char first_byte = m_buff[index];
    int codepoint_len = InputString::get_byte_len(first_byte);
    if (codepoint_len > 4){
        dump_debug(m_buff, state, data, index, "Too long Unicode code point found.", "utf8");
        logger::warn("Too long Unicode code point found.");
        return index;
    }
    char codepoint[5] {first_byte, '\0', '\0', '\0', '\0'};
    for (size_t i = index  + 1; i <= m_buff.size(); i++){
        if (i - (index + 1) >= codepoint_len - 1){
            utf8proc_int32_t cp;
            int consumed = utf8proc_iterate(reinterpret_cast<utf8proc_uint8_t*>(codepoint), 4, &cp);
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
            return i;
        }
        if (i - index < 5){
            codepoint[i - index] = m_buff[i];
        } else {
            logger::warn("Something went wrong");
        }
    }
    logger::warn("Something went wrong");
    return index;
}
static size_t parseInput(string& buffer, State& s, Data& d){
    int idx = 0;
    while (idx < buffer.size()){
        if (d.exit){
            d.exit = false;
            break;
        }
        switch (s){
            case State::Ground:
                idx = ground(buffer, s, d, idx);
                break;
            case State::Utf8:
                idx = utf8(buffer, s, d, idx);
                break;
            case State::Esc:
                idx = esc(buffer, s, d, idx);
                break;
            case State::Ss3:
                idx = ss3(buffer, s, d, idx);
                break;
            case State::Csi:
                idx = csi(buffer, s, d, idx);
                break;
            default:
                logger::error("No function defined for state: " + to_string(static_cast<uint8_t>(s)));
        }
    }
    if (d.has_data()){
        d.push();
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
        char* s = tigetstr(const_cast<char*>(name));
        if (s == (char*)-1 || s == nullptr) return {};
        return std::string(s);
}

void d_default_input::m_setRawMode() {
    tcgetattr(STDIN_FILENO, &m_old);
    termios t = m_old;
    t.c_lflag &= ~(ICANON | ECHO | ISIG);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
    // set to application cursor mode.
    int err = 0;
    if (setupterm(nullptr, STDOUT_FILENO, &err) != 0) {
        logger::critical("[Input] error in setupterm " + to_string(err));
        terminate();
    }

    std::string smkx = m_getCap("smkx");
    std::string rmkx = m_getCap("rmkx");
    if (!smkx.empty()) putp(const_cast<char*>(smkx.c_str()));
}
void d_default_input::m_restoreMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &m_old);
    // reset from application cursor mode.
    std::string rmkx = m_getCap("rmkx");
    if (!rmkx.empty()) putp(const_cast<char*>(rmkx.c_str()));
}
void d_default_input::m_fillDb(){
    if (csi_map.empty() && ss3_map.empty()){
#define KEY(name, id, type, param, last_byte)\
        if (type == static_cast<uint8_t>(KEY_TYPES::CSI)){\
            long long key = make_key(param, static_cast<char>(last_byte));\
            logger::debug("Adding CSI key: " + to_string(Key::name) + " param: " + to_string(param) + " last_byte: " + string(1, static_cast<char>(last_byte)) + " Key is: " + to_string(key));\
            csi_map[key] = Key::name;\
        }\
        else if (type == static_cast<uint8_t>(KEY_TYPES::SS3)){\
            ss3_map[static_cast<char>(last_byte)] = Key::name;\
            logger::debug("Adding SS3 key: " + to_string(Key::name) + " param: " + to_string(param) + " last_byte: " + string(1, static_cast<char>(last_byte)));\
        }\
        else if (type == static_cast<uint8_t>(KEY_TYPES::CTRL)){\
            logger::debug("Adding CTRL key: " + to_string(Key::name) + " param: " + to_string(param) + " last_byte: " + to_string(last_byte));\
            ctrl_map[static_cast<char>(last_byte)] = pair<Key, int>(Key::name, param);\
        }
#include "../../data/keys.def"
#undef KEY
    }
}
// ===========================================
vector<Event> d_default_input::d_getEvents(int timeout_ms){
    m_events.clear();
    if (!checkstdin(timeout_ms)){
        return m_events;
    }
    m_buff += getData();
    size_t idx = parseInput(m_buff, m_state, m_data);
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
