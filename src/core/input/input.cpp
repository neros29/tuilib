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
    set_raw_mode();
    populateDb();
}

Input::~Input() {
    restore_mode();
}

void Input::set_raw_mode() {
    tcgetattr(STDIN_FILENO, &g_old);
    termios t = g_old;
    t.c_lflag &= ~(ICANON | ECHO); // raw-ish: no line buffering, no echo
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void Input::restore_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &g_old);
}
string Input::cap(const char* name){
        char* s = tigetstr(const_cast<char*>(name));
        if (s == (char*)-1 || s == nullptr) return {};
        return std::string(s);
}
void Input::populateDb(){
  int err = 0;
    if (setupterm(nullptr, STDOUT_FILENO, &err) != 0) {
        clog << "error = " << err << endl;
        terminate();
    }

    // Enable keypad transmit / application mode so terminfo key strings match.
    // This is the missing piece for arrows on many terminals.
    std::string smkx = cap("smkx");
    std::string rmkx = cap("rmkx");
    if (!smkx.empty()) putp(const_cast<char*>(smkx.c_str()));


    auto add = [&](const char* capname, const char* human) {
        std::string s = cap(capname);
        if (s.empty()) {
            std::clog << "<missing>\n";
        } else {
            m_termDb[s] = human;
        }
    };

    add("kcuu1", "Up");
    add("kcud1", "Down");
    add("kcub1", "Left");
    add("kcuf1", "Right");
    add("khome", "Home");
    add("kend",  "End");
    add("kdch1", "Delete");
    add("kich1", "Insert");
    add("kpp",   "PageUp");
    add("knp",   "PageDown");
    add("kbs",   "Backspace");
    add("kent",  "Enter");

    for (int i = 1; i <= 12; i++) {
        std::string capname = "kf" + std::to_string(i);
        std::string human = "F" + std::to_string(i);
        std::string s = cap(capname.c_str());
        if (!s.empty()) {
            m_termDb[s] = human;
        }
    }

    
}

int Input::getDataSize(){
    int size;
    ioctl(0, FIONREAD, &size);
    return size;
}
void Input::getData(){
    m_buffer = "";
    int dataSize = getDataSize();
    if (dataSize){
        clog << "data size: " << dataSize << endl;
        char buff[dataSize];
        read(0, &buff, dataSize);
        buff[dataSize] = '\0';
        m_buffer = buff;
    }
}
void Input::parseBuffer(){
    string human;
    getData();
    if (m_buffer.empty()){
        return;
    }
    if ((unsigned char)m_buffer[0] == 0x1b){
        auto it = m_termDb.find(m_buffer);
        if (it != m_termDb.end()){
            human = it->second; 
        } else {
            return;
        }
    } else {
        human = m_buffer; 
    }
    for (int i : m_sortIndex){
        Surface& surf = m_surfaces[i];
        auto it = find(surf.keys.begin(), surf.keys.end(), human);
        if (it != surf.keys.end()){
            surf.events[human] = true;
        }
    }
    

};
void Input::update(){
    parseBuffer();
}
