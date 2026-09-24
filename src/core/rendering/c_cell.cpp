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

bool c_cell::is_def(string type) const{
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
bool c_cell::operator ==(const c_cell chi) const{
    if (chi.m_bg != m_bg){
        return false;
    }
    if (chi.m_fg != m_fg){
        return false;
    }
    if (*chi.ch != *ch){
        return false;
    }
    return true;
}

bool c_cell::operator !=(const c_cell chi) const{
    if (chi.m_bg != m_bg){
        return true;
    }
    if (chi.m_fg != m_fg){
        return true;
    }
    if (*chi.ch != *ch){
        return true;
    }
    return false;
}
