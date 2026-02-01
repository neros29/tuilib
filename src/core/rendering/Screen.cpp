#include <iostream>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <algorithm>
#include <fstream>
#include <deque>

#include "../tui.h"

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
    clog << "[Screen] m_initScreen was called"<< endl;
    m_screen.assign(m_size[0] * m_size[1], m_defaultCharacter);
    m_lastScreen.assign(m_size[0] * m_size[1], m_defaultCharacter);
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
        Character ch;
        for (int j: m_sortIndex){
            const Surface &surf = m_surfaces[j];
            int r_x = x - surf.get_offset()[0];
            int r_y = y - surf.get_offset()[1];
            if (r_x < surf.get_size()[0] && r_y < surf.get_size()[1]){
                if (r_x >= 0 && r_y >= 0){
                    int index = (surf.get_size()[0] * r_y) + r_x;
                    if (!surf[index].ch_def && ch.ch_def){
                        ch.set_ch(surf[index].ch);
                        def = false;
                    }
                    if (!surf[index].fg_def && ch.fg_def){
                        ch.set_fg(surf[index].fg[0], surf[index].fg[1], surf[index].fg[2]);
                        def = false;
                    }
                    if (!surf[index].bg_def && ch.bg_def){
                        ch.set_bg(surf[index].bg[0], surf[index].bg[1], surf[index].bg[2]);
                        def = false;
                    }
                    if (!ch.bg_def && !ch.ch_def){
                        if (!ch.fg_def){
                            ch.genrate();
                            m_screen[i] = ch.ansii;
                            break;

                        }
                    }
                    
                }
            }
        }
        if (def){
            m_screen[i] = m_defaultCharacter;
        }
        else{
            ch.genrate();
            m_screen[i] = ch.ansii;
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


void Screen::flip(){
    m_render();
    m_file << "\x1b[?25l";
    for (int i = m_screen.size()-1; i >= 0; i--){
        int y = i / m_size[0];
        int x = i % m_size[0];
        if (m_screen[i] != m_lastScreen[i]){
            m_file <<"\x1b[" << y << ";"<< x << "H" << m_screen[i];
            amount++;
        }
    }
    m_lastScreen = m_screen;
    m_file <<"\x1b[?25h";
    m_file << "\x1b["<< m_size[0] << ";" << m_size[1] << "H";
    m_file.flush();
}
Screen::Screen(const deque<Surface>& surfaces, const vector<int>& sortIndex):m_surfaces(surfaces), m_sortIndex(sortIndex){
    clog << "[Screen] Screen was Constructed"<< endl;
    m_getWinSize();
    m_initScreen();
    m_file.open("/dev/stdout");
}
Screen::~Screen(){
    clog << "[Screen] Screen's Destructer ran"<< endl;
    m_file.close();
}

