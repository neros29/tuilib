#include <sstream>
#include <string>
#include "../tui.h"

using namespace std;

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
    ch = chi;
}
void Character::set_bg(int r, int g, int b){
    bg[0] = r;
    bg[1] = g;
    bg[2] = b;
    bg_def = false;
}
void Character::set_fg(int r, int g, int b){
    fg[0] = r;
    fg[1] = g;
    fg[2] = b;
    fg_def = false;
}
