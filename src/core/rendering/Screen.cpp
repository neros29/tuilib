#include <iostream>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <fstream>
#include <deque>

#include "../tui.h"
#include "../utils/logging.h"
using namespace std;

void Screen::m_getWinSize(){
    struct winsize ws = {(struct winsize){0,0,0,0}};
    ioctl(0, TIOCGWINSZ, &ws);
    if (m_size[0] != ws.ws_col + 1 || m_size[1] != ws.ws_row + 1){
        m_size[0] = ws.ws_col + 1;
        m_size[1] = ws.ws_row + 1;
        m_sizeChange = true;
    }
    else {
        m_sizeChange = false;
    }
}

array<int, 2> Screen::getSize(){
    return m_size;
}

void Screen::m_initScreen(){
    Character character;
    character.set_ch(m_defaultCharacter);
    m_screen.assign(m_size[0] * m_size[1],character);
    m_lastScreen.assign(m_size[0] * m_size[1], character);
}

void Screen::m_render(){
    m_getWinSize();
    if (m_sizeChange){
        m_initScreen();
    }
    int x = 1;
    int y = 1;
    for (int i = 0; i != m_screen.size(); i++){
        bool def {true};
        Character newCharacter;
        for (int j: m_sortIndex){
            const InternalSurface &surf = m_surfaces[j];
            int r_x = x - surf.r_cords[0];
            int r_y = y - surf.r_cords[1];
            if (r_x < surf.size[0] && r_y < surf.size[1]){
                if (r_x >= 0 && r_y >= 0){
                    int index = (surf.size[0] * r_y) + r_x;
                    const Character& currentCharacter = surf[index];
                    if (!currentCharacter.ch_def && newCharacter.ch_def){
                        newCharacter.set_ch(currentCharacter.ch);
                        def = false;
                    }
                    if (!currentCharacter.fg_def && newCharacter.fg_def){
                        newCharacter.set_fg(currentCharacter.fg[0], currentCharacter.fg[1], currentCharacter.fg[2]);
                        def = false;
                    }
                    if (!currentCharacter.bg_def && newCharacter.bg_def){
                        newCharacter.set_bg(currentCharacter.bg[0], currentCharacter.bg[1], currentCharacter.bg[2]);
                        def = false;
                    }
                    if (!newCharacter.bg_def && !newCharacter.ch_def){
                        if (!newCharacter.fg_def){
                            break;
                        }
                    }
                }
            }
        }
        if (def){
            newCharacter.set_ch(m_defaultCharacter);
            m_screen[i] = newCharacter;
        }
        else{
            m_screen[i] = newCharacter;
        }
        if (x == m_size[0]){
            x = 1;
            y ++;
        }
        else{
            x++;
        }
    }
}

string Screen::m_genrate(Character& ch){
    static string ESC {"\x1B"};
    static string colors[256];
    if (colors[1].empty()){
        logger::info("[Character] Creating colors cache");
        for (int i = 0; i <= 255; i++){
            colors[i] = to_string(i);
        }
    }
    string ansii;
    ansii.clear();
    if (!ch.fg_def){
        ansii += ESC + "[38;2;" + colors[ch.fg[0]] + ";" + colors[ch.fg[1]] + ";" + colors[ch.fg[2]]  + "m";
    }
    if (!ch.bg_def){
        ansii += ESC + "[48;2;"+ colors[ch.bg[0]] + ";" + colors[ch.bg[1]] + ";" + colors[ch.bg[2]] + "m";
    }
    ansii += ch.ch + ESC + "[0m";
    return ansii;
}

void Screen::flip(){
    m_render();
    m_file << "\x1b[?25l";
    for (int i = m_screen.size()-1; i >= 0; i--){
        int y = i / m_size[0];
        int x = i % m_size[0];
        if (m_screen[i] == m_lastScreen[i]){
            continue;
        }
        else {
            m_file <<"\x1b[" << y << ";"<< x << "H" << m_genrate(m_screen[i]);
            charactersRenderd++;
        }
    }
    m_lastScreen = m_screen;
    m_file <<"\x1b[?25h";
    m_file << "\x1b["<< m_size[0] << ";" << m_size[1] << "H";
    m_file.flush();
}

Screen::Screen(const deque<InternalSurface>& surfaces, const vector<int>& sortIndex):m_surfaces(surfaces), m_sortIndex(sortIndex){
    logger::info("[Screen] Starting up");
    m_getWinSize();
    m_initScreen();
    m_file.open("/dev/stdout");
}

Screen::~Screen(){
    m_file.close();
}
