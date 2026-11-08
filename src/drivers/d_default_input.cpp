#include "../core/drivers.h"
#include <poll.h>
#include <string>
#include <unistd.h>
#include <termios.h>

using namespace drivers;
using namespace std;
using namespace core;

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
    int dataSize = getDataSize();
    if (dataSize){
        char buff[dataSize + 1];
        read(0, &buff, dataSize);
        buff[dataSize] = '\0';
        return buff;
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
void d_default_input::m_add(std::string key, Key value){
    if (key.empty()){
        logger::info("Key: " + to_string((uint32_t)value) + " Dose not exist in terminfo.");
        return;
    }
    logger::debug("Added key: " + to_string((uint32_t)value));
    m_keydb[key] = value;
}
void d_default_input::m_fillDb(){
    m_add(m_getCap("kcuu1"), Key::Up);
    m_add(m_getCap("kcud1"), Key::Down);
    m_add(m_getCap("kcub1"), Key::Left);
    m_add(m_getCap("kcuf1"), Key::Right);
    // arrow with modifiers
    m_add(m_getCap("cuu1"), Key::Up);
    m_add(m_getCap("cud1"), Key::Down);
    m_add(m_getCap("cub1"), Key::Left);
    m_add(m_getCap("cuf1"), Key::Right);

    m_add(m_getCap("khome"), Key::Home);
    // with modifiers
    m_add(m_getCap("home"), Key::Home);

    m_add(m_getCap("kend"),  Key::End);

    m_add(m_getCap("kdch1"), Key::Delete);
    m_add(m_getCap("kich1"), Key::Insert);
    m_add(m_getCap("kpp"),   Key::PageUp);
    m_add(m_getCap("knp"),   Key::PageDown);
    m_add(m_getCap("kbs"),   Key::Backspace);
    // f keys.
    m_add(m_getCap("kf1"),   Key::F1);
    m_add(m_getCap("kf2"),   Key::F2);
    m_add(m_getCap("kf3"),   Key::F3);
    m_add(m_getCap("kf4"),   Key::F4);
    m_add(m_getCap("kf5"),   Key::F5);
    m_add(m_getCap("kf6"),   Key::F6);
    m_add(m_getCap("kf7"),   Key::F7);
    m_add(m_getCap("kf8"),   Key::F8);
    m_add(m_getCap("kf9"),   Key::F9);
    m_add(m_getCap("kf10"),  Key::F10);
    m_add(m_getCap("kf11"),  Key::F11);
    m_add(m_getCap("kf12"),  Key::F12);
}
void d_default_input::m_addCtrl(char k, char v){
    logger::debug("[d_default_input][fillCtrldb] Added Ctrl + " + string() + v + " key with Byte: " + to_string(k));
    m_asciiCtrldb[k] = v;
}
void d_default_input::m_fillCtrldb(){
    // genraly speaking last character wins. I am still leaving every character even ones that match keys, just for completeness.
    m_addCtrl(0, '@');  
    m_addCtrl(0, ' ');
    m_addCtrl(0, '2');

    std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
    for (int i = 1; i <= 26; i ++){ // add the alphabet since it's a simple order
        m_addCtrl(i, alphabet[i - 1]);
    }
    m_addCtrl(27, '[');  
    m_addCtrl(27, '3');  

    m_addCtrl(28, '\\');

    m_addCtrl(29, ']');  
    m_addCtrl(29, '5');  

    m_addCtrl(30, '~');
    m_addCtrl(30, '^');  
    m_addCtrl(30, '6');  

    m_addCtrl(31, '_');
    m_addCtrl(31, '/');
    m_addCtrl(31, '7');

    m_addCtrl(48, '0');  
    m_addCtrl(49, '1');
    m_addCtrl(57, '9');  

    // 127
    m_addCtrl(127, '?');
    m_addCtrl(127, '8');  
}
// ===========================================
vector<Event> d_default_input::d_getEvents(int timeout_ms){
    vector<Event> results {};
    if (!checkstdin(timeout_ms)){
        return results;
    }
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
                results.push_back(Event{Kind::Text, modifers::alt, string() + ch});
                event_buff.clear();
            } 
            was_esc = false;
            continue;
        }
        else if (in_esc){
            if (ch == ';'){
                if (event_buff[event_buff.size() - 1] == '1'){
                    event_buff.pop_back();
                }
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
                if (event_buff[1] != ss3 && event_buff[1] != csi){
                    continue;
                }
            }
            auto it = m_keydb.find(event_buff);
            if (it != m_keydb.end()){
                logger::debug("[d_default_input][getEvents] Key: modifer: "  + to_string((uint8_t)modifer) + " key: " + to_string((uint32_t)it->second));
                results.push_back(Event{Kind::Key, modifer, "", it->second});
            } else {
                logger::debug("[d_default_input][getEvents] Key: '" + event_buff + "' was not found.");
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
            results.push_back(Event{Kind::Text, modifer, event_buff});
            event_buff.clear();
        } else if (auto it = m_asciiCtrldb.find(ch); it != m_asciiCtrldb.end()){
            logger::debug("Ctrl key hit: " + string() + it->second);
            results.push_back(Event{Kind::Text, modifer | modifers::ctrl, string() + it->second});
        }
        else {
            std::string lower{ch};
            if ('A' <= ch && ch <= 'Z'){
                modifer = modifers::shift;
                lower = ch + ('a' - 'A');
            }
            results.push_back(Event{Kind::Text, modifer, lower});
        }
    }
    m_buff.clear();
    m_buff += event_buff;
    return results;
}
d_default_input::d_default_input(){
    m_setRawMode();
    m_fillDb();
    m_fillCtrldb();
}
d_default_input::~d_default_input(){
    m_restoreMode();
}
