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
    string ESC {"\x1B"};
    ostringstream oss;
    ostringstream _fg;
    ostringstream _bg;
    if (!fg_def){
        _fg << ESC << "[38;2;"<< fg[0] << ";" << fg[1] << ";"<< fg[2] <<"m";
    }
    if (!bg_def){
        _bg << ESC << "[48;2;"<< bg[0] << ";" << bg[1] << ";" << bg[2] << "m";
    }
    oss << _bg.str() << _fg.str() << ch << ESC << "[0m";
    ansii = oss.str();
}
void Character::set_ch(string chi){
    if (chi.empty()){
        throw invalid_argument{"string must have some value"};
    }
    ch = chi;
    ch_def = false;
}
void Character::set_bg(int r, int g, int b){
    cheakInput(r, g, b);
    bg[0] = r;
    bg[1] = g;
    bg[2] = b;
    bg_def = false;
}
void Character::set_fg(int r, int g, int b){
    cheakInput(r, g, b);
    fg[0] = r;
    fg[1] = g;
    fg[2] = b;
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
