#include "../tui.h"
#include "../utils/logging.h"
#include <algorithm>
#include <string>

using namespace core;

c_surface* c_surfaces::add(std::array<int, 2> size, std::string ch, int z, std::array<int, 2> offset, const std::vector<std::string>& keys){
    m_surfaces.emplace_back(size, ch, z, offset, keys, m_nextId);
    m_nextId ++;
    c_surface* new_surf = &m_surfaces[m_surfaces.size()-1];
    m_sortedSurfaces.push_back(new_surf);
    return new_surf;
}
void c_surfaces::sort(){
    std::sort(m_sortedSurfaces.begin(), m_sortedSurfaces.end(), 
        [](const c_surface* a, const c_surface* b){
        
            return a->z > b->z;
        }
    );
}
size_t c_surfaces::size(){
    return m_surfaces.size();
}
c_surfaces::SurfaceIterator c_surfaces::begin() {
    return m_sortedSurfaces.begin();
}

c_surfaces::SurfaceIterator c_surfaces::end() {
    return m_sortedSurfaces.end();
}
