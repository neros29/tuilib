#include "../api.h"
#include "../core/utils/logging.h"
#include "../core/drivers.h"

using namespace std;
using namespace core;
Surface Tui::addSurface(array<int, 2> size,  array<int, 2> offset, string ch, int z){
    
    if (z == -1){
        z = m_index;
    }
    c_surface* surf = m_surfaces.add(size, ch, z, offset, m_input.keys);
    m_index ++;
    return Surface(surf);
}
Tui::Tui():m_screen(m_screenDriver), m_input(), m_screenDriver(new drivers::d_defualt_screen){
    logger::info("[API] Starting up");
}
Tui::~Tui(){
    delete m_screenDriver;
    logger::debug("[Tui] destructer run.");
}


void Tui::update(){
    bool isDirty = m_input.update(m_surfaces);
    m_surfaces.sort();
    // isDirty = true;
    for (c_surface& surf: m_surfaces){
        if (surf.cheakDirty()){
            isDirty = true;
        }
    }
    if (isDirty){
        m_screen.flip(m_surfaces);
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
