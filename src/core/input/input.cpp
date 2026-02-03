#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <iostream>
#include <algorithm>
#include "../tui.h"
#include <term.h>

using namespace std;

Input::Input(deque<Surface>& surfaces, const vector<int>& sortIndex):m_surfaces(surfaces), m_sortIndex(sortIndex){
    clog << "[Input] Starting up" << endl;
    m_setRawMode();
    m_initNamePair();
    m_initDb();
}
Input::~Input() {
    clog << "[Input] Shuting down" << endl;
    m_restoreMode();
}

inline void Input::m_initNamePair(){
    for (int i = 1; i <= 12; i++) {
        std::string capname = "kf" + std::to_string(i);
        std::string human = "F" + std::to_string(i);
        m_namePair.push_back({capname, human});
    }
    m_namePair.push_back({"kcuu1", "Up"});
    m_namePair.push_back({"kcud1", "Down"});
    m_namePair.push_back({"kcub1", "Left"});
    m_namePair.push_back({"kcuf1", "Right"});
    m_namePair.push_back({"khome", "Home"});
    m_namePair.push_back({"kend",  "End"});
    m_namePair.push_back({"kdch1", "Delete"});
    m_namePair.push_back({"kich1", "Insert"});
    m_namePair.push_back({"kpp",   "PageUp"});
    m_namePair.push_back({"knp",   "PageDown"});
    m_namePair.push_back({"kbs",   "Backspace"});


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
void Input::m_initDb(){
    int err = 0;
    if (setupterm(nullptr, STDOUT_FILENO, &err) != 0) {
        clog << "[Input] error = " << err << endl;
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
            clog << "[Input] "<< capname << " dose not exist in terminfo " << endl;
        } else {
            m_termDb[s] = human;
        }
    }
}

int Input::m_getDataSize(){
    int size;
    ioctl(0, FIONREAD, &size);
    return size;
}

void Input::m_getData(){
    m_buffer = "";
    int dataSize = m_getDataSize();
    if (dataSize){
        char buff[dataSize + 1];
        read(0, &buff, dataSize);
        buff[dataSize] = '\0';
        m_buffer = buff;
    }
}

bool Input::m_parseBuffer(){
    string human;
    m_getData();
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
    for (int i : m_sortIndex){
        Surface& surf = m_surfaces[i];
        auto it = find(surf.keys.begin(), surf.keys.end(), human);
        if (it != surf.keys.end()){
            surf.events[human] = true;
            return true;
        }
    }
    return false;
};
bool Input::update(){
    return m_parseBuffer();
}
