#include <array>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <algorithm>
#include <term.h>
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
    t.c_lflag &= ~(ICANON | ECHO);
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

bool Input::update(c_surfaces& surfs){
    string human;
    m_buffer = getData();
    if (m_buffer.empty()){
        return false;
    }
    if ((unsigned char)m_buffer[0] == 0x1b){
        auto it = m_termDb.find(m_buffer);
        if (it != m_termDb.end()){
            human = it->second; 
        } else {
            return false;
        }
    } else {
        human = m_buffer; 
    }
    for (c_surface& surf : surfs){
        auto it = find(surf.keys.begin(), surf.keys.end(), human);
        if (it != surf.keys.end()){
            surf.events[human] = true;
            return true;
        }
    }
    return false;
};
