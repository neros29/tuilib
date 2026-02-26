#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <sys/ioctl.h>
#include "../tui.h"
#include <cassert>

using namespace std;

inline void cheakVariables(const array<int, 2>& size, const string& ch){
    if (size[0] < 0 || size[1] < 0){
        throw invalid_argument{"Size must be positive"};
    }
    if (ch.empty()){
        throw invalid_argument{"Defutlt Character must not be empty"};
    }
}
InternalSurface::InternalSurface(array<int, 2> size, string ch, int z, array<int, 2> offset, const vector<string>& keys):z(z), r_cords{offset[0], offset[1]}, size{size[0], size[1]}, m_validKeys(&keys){
    cheakVariables(size, ch);
    Character chr;
    chr.set_ch(ch);
    chr.genrate();
    surface.assign(size[0]*size[1], chr);
}
InternalSurface::InternalSurface(array<int, 2> size, string ch, int z, array<int, 2> offset):z(z), r_cords{offset[0], offset[1]}, size{size[0], size[1]}, m_validKeys(nullptr){
    cheakVariables(size, ch);
    Character chr;
    chr.set_ch(ch);
    chr.genrate();
    surface.assign(size[0]*size[1], chr);
}

bool InternalSurface::cheakDirty(){
    if (m_old != surface){
        m_old = surface;
        return true;
    }
    if (m_dirty){
        m_dirty = false;
        return true;
    }
    return false;
}

Character& InternalSurface::operator[](int x){
    return surface.at(x);
}
Character InternalSurface::operator[](int x) const{
    return surface.at(x);
}

