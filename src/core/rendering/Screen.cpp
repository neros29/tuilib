#include <array>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <fstream>
#include <deque>
#include <cmath> // for std::round

#include "../tui.h"
#include "../utils/logging.h"

using namespace std;
using namespace core;
/**
 * Blends two RGBA color arrays.
 * @param src The foreground color array [R, G, B, A] (0-255)
 * @param dst The background color array [R, G, B, A] (0-255)
 * @return A new 4-element array containing the blended color.
 */
static array<int, 4> blendColors(const std::array<int, 4>& src, const std::array<int, 4>& dst) {
    if (src[3] <= 0){
        return dst;
    }
    else if (dst[3] <= 0){
        return src;
    }
    array<int, 4> result;

    // Convert alpha to 0.0 - 1.0 range for math
    float alpha_src = src[3] / 255.0f;
    float alpha_dst = dst[3] / 255.0f;
    float inv_alpha_src = 1.0f - alpha_src;

    // 1. Blend RGB channels
    // Formula: C_out = (C_src * alpha_src) + (C_dst * (1 - alpha_src))
    result[0] = static_cast<int>(round(src[0] * alpha_src + dst[0] * inv_alpha_src));
    result[1] = static_cast<int>(round(src[1] * alpha_src + dst[1] * inv_alpha_src));
    result[2] = static_cast<int>(round(src[2] * alpha_src + dst[2] * inv_alpha_src));

    // 2. Accumulate the Alpha channel
    // Formula: A_out = A_src + A_dst * (1 - A_src)
    float alpha_out = alpha_src + (alpha_dst * inv_alpha_src);
    result[3] = static_cast<int>(round(alpha_out * 255.0f));

    return result;
}

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
        array<int, 4> bg {0, 0, 0, 0};
        array<int, 4> fg {0, 0, 0, 0};
        Character newCharacter;
        for (int j: m_sortIndex){
            const c_surface &surf = m_surfaces[j];
            int r_x = x - surf.r_cords[0];
            int r_y = y - surf.r_cords[1];
            if (r_x < surf.size[0] && r_y < surf.size[1]){
                if (r_x >= 0 && r_y >= 0){
                    int index = (surf.size[0] * r_y) + r_x;
                    const Character& currentCharacter = surf[index];

                    if (!currentCharacter.is_def("ch") && newCharacter.is_def("ch") && currentCharacter.get_ch() != m_defaultCharacter){
                        newCharacter.set_ch(currentCharacter.get_ch());
                        fg = blendColors(bg, currentCharacter.get_fg());
                        newCharacter.set_fg(fg[0], fg[1], fg[2], fg[3]);
                    } 

                    bg = blendColors(newCharacter.get_bg(), currentCharacter.get_bg());
                    newCharacter.set_bg(bg[0], bg[1], bg[2], bg[3]);

                    if (bg[3] >= 255 && !newCharacter.is_def("ch")){
                        break;
                    }
                }
            }
        }
        m_screen[i] = newCharacter;
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
    ansii = "\x1b[0m";

    array<int, 4> fg = ch.get_fg();
    if (!ch.is_def("fg")){
        ansii += ESC + "[38;2;" + colors[fg[0]] + ";" + colors[fg[1]] + ";" + colors[fg[2]]  + "m";
    }
    array<int, 4> bg = ch.get_bg();
    if (bg[3] > 0 && !ch.is_def("bg")){
        ansii += ESC + "[48;2;"+ colors[bg[0]] + ";" + colors[bg[1]] + ";" + colors[bg[2]] + "m";
    }

    if (!ch.is_def("ch") && fg[3] > 0){
        ansii += ch.get_ch();
    } else{
        ansii += m_defaultCharacter;
    }

    return ansii;
}

void Screen::flip(){
    m_render();
    m_file << "\x1b[?25l";
    bool was_unicode = false;
    for (int i = 0; i < m_screen.size(); i++){
        int y = i / m_size[0];
        int x = i % m_size[0];
        if (was_unicode){
            was_unicode = false;
            continue;
        }
        if (m_screen[i] != m_lastScreen[i]){
            m_file <<"\x1b[" << y << ";"<< x << "H" << m_genrate(m_screen[i]);
            charactersRenderd++;
        } 
        was_unicode = InputString::getWidth(m_screen[i].get_ch()) > 1;
    }
    m_lastScreen.swap(m_screen);
    m_file << "\x1b[0m";
    m_file <<"\x1b[?25h";
    m_file << "\x1b["<< m_size[0] << ";" << m_size[1] << "H";
    m_file.flush();
}

Screen::Screen(const deque<c_surface>& surfaces, const vector<int>& sortIndex):m_surfaces(surfaces), m_sortIndex(sortIndex){
    logger::info("[Screen] Starting up");
    m_getWinSize();
    m_initScreen();
    m_file.open("/dev/stdout");
}

Screen::~Screen(){
    m_file.close();
}
