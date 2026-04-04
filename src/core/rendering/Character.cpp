#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "../tui.h"

using namespace std;

inline void cheakInput(int r, int g, int b){
    if (r > 255 || g > 255 || b > 255){
        throw invalid_argument{"colors must be bettween 0 and 255"};
    }
    if (r < 0 || g < 0 || b < 0){
        throw invalid_argument{"colors must be none negative"};
    }
}

void Character::genrate(){
    static string ESC {"\x1B"};
    static string colors[256];
    if (colors[1].empty()){
        clog << "[Character] Creating colors cache" << endl;
        for (int i = 0; i <= 255; i++){
            colors[i] = to_string(i);
        }
    }
    if (!defualt){
        ansii.clear();
        if (!fg_def){
            ansii += ESC + "[38;2;" + colors[fg[0]] + ";" + colors[fg[1]] + ";" + colors[fg[2]]  + "m";
        }
        if (!bg_def){
            ansii += ESC + "[48;2;"+ colors[bg[0]] + ";" + colors[bg[1]] + ";" + colors[bg[2]] + "m";
        }
        ansii += ch + ESC + "[0m";
        defualt = true;
    }
}
void Character::set_ch(string chi){
    if (chi.empty()){
        throw invalid_argument{"string must have some value"};
    }
    ch = chi;
    defualt = false;
    ch_def = false;
}
void Character::set_bg(int r, int g, int b){
    cheakInput(r, g, b);
    bg[0] = r;
    bg[1] = g;
    bg[2] = b;

    defualt = false;
    bg_def = false;
}
void Character::set_fg(int r, int g, int b){
    cheakInput(r, g, b);
    fg[0] = r;
    fg[1] = g;
    fg[2] = b;
    defualt = false;
    fg_def = false;
}
bool Character::operator ==(const Character chi) const{
    if (chi.bg != bg){
        return false;
    }
    if (chi.fg != fg){
        return false;
    }
    if (chi.ch != ch){
        return false;
    }
    return true;
}
