#include <array>
#include <stdexcept>
#include <string>
#include "../tui.h"

using namespace std;
using namespace core;

inline void cheakInput(int r, int g, int b, int a){
    if (r > 255 || g > 255 || b > 255 || a > 255){
        throw invalid_argument{"colors must be bettween 0 and 255"};
    }
    if (r < 0 || g < 0 || b < 0 || a < 0){
        throw invalid_argument{"colors must be none negative"};
    }
}

void Character::set_ch(string chi){
    ch = chi;
    ch_def = false;
}

bool Character::is_def(string type) const{
    if (type == "ch"){
        return ch_def;
    }
    else if (type == "fg"){
        return fg_def;
    }

    else if (type == "bg"){
        return bg_def;
    }
    else {
        throw invalid_argument{"You must pass eather ch, fg, or bg into get_def."};
    }
}
void Character::set_bg(int r, int g, int b, int a){
    cheakInput(r, g, b, a);
    bg[0] = r;
    bg[1] = g;
    bg[2] = b;
    bg[3] = a;
    bg_def = false;
}
void Character::set_fg(int r, int g, int b, int a){
    cheakInput(r, g, b, 0);
    fg[0] = r;
    fg[1] = g;
    fg[2] = b;
    fg[3] = a;
    fg_def = false;
}
string Character::get_ch() const{
    return ch;
}
array<int, 4> Character::get_bg() const{
    return bg;
}
array<int, 4> Character::get_fg() const{
    return fg;
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

bool Character::operator !=(const Character chi) const{
    if (chi.bg != bg){
        return true;
    }
    if (chi.fg != fg){
        return true;
    }
    if (chi.ch != ch){
        return true;
    }
    return false;
}
