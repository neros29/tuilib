#include "../api.h"
#include <algorithm>
#include <iostream>

using namespace std;

Surface &Tui::append(array<int, 2> size,  array<int, 2> offset, string ch, int z){
    if (z == -1){
        z = m_index;
    }
    m_surfaces.emplace_back(size, ch, z, offset);
    m_sortedSurfaceIndex.emplace_back(m_index);
    m_index ++;
    m_isSurfaceSorted = false;
    return m_surfaces[m_index-1];
}

Tui::Tui():m_screen(Screen(m_surfaces, m_sortedSurfaceIndex)), m_input(Input(m_surfaces, m_sortedSurfaceIndex)), m_index(0){
    clog << "[API] Starting up" << endl;
}

Tui::~Tui(){
    clog << "[API] Shutting down" << endl;
}

void Tui::m_sortSurfaces(){
    sort(m_sortedSurfaceIndex.begin(), m_sortedSurfaceIndex.end(), 
        [&](const int& a, const int& b){
            return m_surfaces[a].get_z() > m_surfaces[b].get_z();
        }
    );
    m_isSurfaceSorted = true;
}

void Tui::update(){
    bool isDirty = m_input.update();
    m_sortSurfaces();
    for (Surface& surf: m_surfaces){
        if (surf.cheakDirty()){
            isDirty = true;
        }
    }
    if (isDirty){
        m_screen.flip();
    }
    else{
        skipedFrames += 1;
    }
}

array<int, 2> Tui::getScreenSize(){
    return m_screen.getSize();
}

int Tui::charactersRendered(){
    return m_screen.charactersRenderd;
}
