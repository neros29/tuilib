#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <algorithm>
#include <term.h>
#include <stdexcept>
#include <vector>
#include "../tui.h"
#include "../utils/logging.h"

using namespace std;
using namespace core;
// static helper functions
static int getDataSize(){
    int size;
    ioctl(0, FIONREAD, &size);
    return size;
}

static string getData(){
    int dataSize = getDataSize();
    if (dataSize){
        char buff[dataSize + 1];
        read(0, &buff, dataSize);
        buff[dataSize] = '\0';
        return buff;
    }
    return "";
}

// Input memeber functinos
Input::Input(){
    logger::info("[Input] Starting up");
    m_setRawMode();
    // cout << "\r\x1b[6n\x1b]66;w=2; \a\x1b[6n\x1b]66;s=2; \a\x1b[6n\x1b[6n";
    // cout.flush();
    //
    // std::vector<std::array<string, 2>> pos;
    // string buff = "";
    // while (pos.size() < 3) {
    //     buff += getData();
    //
    //     // Look for the start of the sequence: \x1b[
    //     size_t start_pos = buff.find("\x1b[");
    //     // Look for the end of the sequence: R
    //     size_t end_pos = buff.find('R', start_pos);
    //
    //     if (start_pos != std::string::npos && end_pos != std::string::npos) {
    //         // We found a full sequence! Let's parse the content between '[' and ';' and 'R'
    //         // Format expected: \x1b[row;colR
    //         size_t semi_colon = buff.find(';', start_pos);
    //
    //         if (semi_colon != std::string::npos && semi_colon < end_pos) {
    //             std::string row = buff.substr(start_pos + 2, semi_colon - (start_pos + 2));
    //             std::string col = buff.substr(semi_colon + 1, end_pos - (semi_colon + 1));
    //
    //             pos.push_back({row, col});
    //
    //             // Clear the processed part of the buffer
    //             buff.erase(0, end_pos + 1);
    //         } else {
    //             // If we found the start but the format is incomplete,
    //             // we might need to wait for more data.
    //             // We'll let the loop continue.
    //         }
    //     }
    // }
    //
    // for (const auto& p : pos) {
    //     logger::debug("x: " + p[0] + " y: " + p[1]);
    // }
    m_initDb();
}
Input::~Input() {
    m_restoreMode();
}
void Input::m_add(string capname, string human){
    keys.push_back(human);
    m_namePair.push_back({capname, human});
}

inline void Input::m_initNamePair(){
    for (int i = 1; i <= 12; i++) {
        std::string capname = "kf" + std::to_string(i);
        std::string human = "F" + std::to_string(i);
        m_add(capname, human);
    }
    m_add("kcuu1", "Up");
    m_add("kcud1", "Down");
    m_add("kcub1", "Left");
    m_add("kcuf1", "Right");
    m_add("khome", "Home");
    m_add("kend",  "End");
    m_add("kdch1", "Delete");
    m_add("kich1", "Insert");
    m_add("kpp",   "PageUp");
    m_add("knp",   "PageDown");
    m_add("kbs",   "Backspace");
}

void Input::m_setRawMode() {
    tcgetattr(STDIN_FILENO, &m_old);
    termios t = m_old;
    t.c_lflag &= ~(ICANON | ECHO | ISIG);
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}
void Input::m_restoreMode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &m_old);
}
string Input::m_getCap(const char* name){
        char* s = tigetstr(const_cast<char*>(name));
        if (s == (char*)-1 || s == nullptr) return {};
        return std::string(s);
}
inline void Input::m_initDb(){
    m_initNamePair();
    int err = 0;
    if (setupterm(nullptr, STDOUT_FILENO, &err) != 0) {
        logger::critical("[Input] error in setupterm " + to_string(err));
        terminate();
    }

    std::string smkx = m_getCap("smkx");
    std::string rmkx = m_getCap("rmkx");
    if (!smkx.empty()) putp(const_cast<char*>(smkx.c_str()));

    for (int i = 0; i < m_namePair.size(); i++){
        array<string, 2>& pair = m_namePair[i]; 
        string human = pair[1];
        string capname = pair[0];
        string s = m_getCap(capname.c_str());
        if (s.empty()) {
            logger::info("[Input] " + capname + " dose not exist in terminfo ");
           ;
        } else {
            m_termDb[s] = human;
        }
    }
}
vector<Event> Input::m_parseData(int timeout_ms){
    vector<Event> results {};
    struct pollfd stdin_fd = {0, POLLIN, 0};
    int code = poll(&stdin_fd, 1, timeout_ms);
    if (code == -1){
        throw invalid_argument{"Error: poll raised error " + to_string(errno) + "."};
    }
    else if (code == 0){
        return results;
    }
    if (!(stdin_fd.revents & POLLIN)){
        throw invalid_argument{"Error: poll raised error " + to_string(errno) + "."};
    }
    // while (true){
    //     char c_buff[1024] = "";
    //     int c = read(0, c_buff, 1024);
    //     if (c == -1){
    //         throw invalid_argument{"Error: read raised error " + to_string(errno) + "."};
    //     } else if (c == 0){
    //         break;
    //     }
    //     c_buff[c] = '\0';
    //     m_buff.append(c_buff);
    // }
    m_buff += getData();
    char esc = '\x1b';
    char ss3 = '\x4f';
    char csi = '\x5b';
    bool was_esc {false};
    bool in_esc {false};
    bool was_mod {false};
    static int wait {0};
    std::string possable_ends = {"ABCDEFHPQRS~"};
    std::string event_buff;
    modifers modifer = modifers::none;
    for (char& ch : m_buff){
        if (wait > 0){
            event_buff += ch;
            wait --;
            continue;
        }
        if (was_esc){
            if (ch == ss3){
                in_esc = true;
                event_buff += ch;
            } else if (ch == csi){
                in_esc = true;
                event_buff += ch;
            } else{
                results.push_back(Event{Event::Kind::Text, modifers::alt, string() + ch});
                event_buff.clear();
            } 
            was_esc = false;
            continue;
        }
        else if (in_esc){
            if (ch == ';'){
                was_mod = true;
                continue;
            }
            if (was_mod){
                modifer = static_cast<modifers>(stoi(string() + ch) - 1);
                was_mod = false;
                continue;
            }
            size_t i = possable_ends.find(ch);
            event_buff += ch;
            if (i >= possable_ends.size()){
                continue;
            }
            else if (possable_ends[i] == '~'){
                if (event_buff[1] != csi){
                    continue;
                }
            }
            else if (possable_ends[i] == 'R'){
                if (event_buff[1] != ss3){
                    continue;
                }
            } else if (i < possable_ends.size()){
                if (event_buff[1] != ss3 && (event_buff[1] != csi || event_buff[2] != '1')){
                    continue;
                }
            }
            auto it = m_termDb.find(event_buff);
            if (it != m_termDb.end()){
                results.push_back(Event{Event::Kind::Text, modifer, it->second});
            }
            event_buff.clear();
            in_esc = false;
            modifer = modifers::none;
        }
        else if (ch == esc){
            was_esc = true;
            event_buff += ch;
        } 
        else if (InputString::get_byte_len(static_cast<unsigned char>(ch)) > 1){
            event_buff += ch;
            wait = InputString::get_byte_len(static_cast<unsigned char>(ch)) - 2;
        } 
        else if (!event_buff.empty()){
            event_buff += ch;
            results.push_back(Event{Event::Kind::Text, modifer, event_buff});
            event_buff.clear();
        }
        else {
            std::string lower{ch};
            if ('A' <= ch && ch <= 'Z'){
                modifer = modifers::shift;
                lower = ch + ('a' - 'A');
            }
            results.push_back(Event{Event::Kind::Text, modifer, lower});
        }
    }
    m_buff.clear();
    m_buff += event_buff;
    return results;
}
bool Input::update(c_surfaces& surfs, int timeout_ms){
    string human;
    vector<Event> events= m_parseData(timeout_ms);
    for (Event e : events){
        logger::info("Event: kind == Text " + std::to_string(e.kind == Event::Kind::Text) + " text '" + e.text + "' key:" + std::to_string((uint32_t)e.key) + " modifer:" + std::to_string((uint8_t)e.modifer));
        for (c_surface& surf : surfs){
            auto it = find(surf.keys.begin(), surf.keys.end(), e.text);
            if (it != surf.keys.end()){
                surf.events[e.text] = true;
            }
        }
    }
    return events.empty();
};
