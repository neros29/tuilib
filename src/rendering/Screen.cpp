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

void Screen::m_sortSurfaces(){
    sort(m_sortIndex.begin(), m_sortIndex.end(), 
        [&](const int& a, const int& b){
            return m_surfaces[a].get_z() > m_surfaces[b].get_z();
        }
    );
    m_sorted = true;
}
array<int, 2> Screen::getSize(){
    return m_size;
}

void Screen::m_initScreen(){
    m_screen.assign(m_size[0] * m_size[1], m_defaultCharacter);
    m_lastScreen.assign(m_size[0] * m_size[1], m_defaultCharacter);
}

void Screen::m_render(){
    m_getWinSize();
    m_sortSurfaces();
    // if (!sorted){
    //     sort_surfaces();
    // }
    if (m_sizeChange){
        m_initScreen();
    }
    int x = 0;
    int y = 0;
    for (int i = 0; i != m_screen.size(); i++){
        bool def {true};
        for (int j: m_sortIndex){
            Surface &surf = m_surfaces[j];
            int r_x = x - surf.offset()[0];
            int r_y = y - surf.offset()[1];
            if (r_x < surf.ssize()[0] && r_y < surf.ssize()[1]){
                if (r_x >= 0 && r_y >= 0){
                    int index = (surf.ssize()[0] * r_y) + r_x;
                    if (surf[index].ch_def){
                        continue;
                    }
                    else{
                        m_screen[i] = surf[index].ansii;
                        def = false;
                        break;
                    }
                    
                }
            }
        }
        if (def){
            m_screen[i] = m_defaultCharacter;
        }
        if (x == m_size[0] - 1){
            x = 0;
            y ++;
        }
        else{
            x++;
        }
    }
}

Surface &Screen::append(array<int, 2> size,  array<int, 2> offset, string ch, int z){
    if (z == -1){
        z = m_index;
    }
    m_surfaces.emplace_back(size, ch, z, offset);
    m_sortIndex.emplace_back(m_index);
    m_index ++;
    m_sorted = false;
    return m_surfaces[m_index-1];
}

void Screen::flip(){
    m_render();
    m_file << "\x1b[?25l";
    for (int i = 0; i != m_screen.size(); i++){
        int y = i / m_size[0];
        int x = i % m_size[0];
        if (m_screen[i] != m_lastScreen[i]){
            m_file <<"\x1b[" << y << ";"<< x << "H" << m_screen[i];
            amount++;
        }
    }
    m_lastScreen = m_screen;
    m_file <<"\x1b[?25h";
    m_file.flush();
}
Screen::Screen(){
    m_getWinSize();
    m_initScreen();
    m_file.open("/dev/stdout");
}
Screen::~Screen(){
    m_file.close();
}

