#include "../api.h"
#include "../core/utils/logging.h"
#include "../core/drivers.h"
#include <cstddef>

using namespace std;
using namespace core;
Surface Tui::addSurface(array<size_t, 2> size,  array<int, 2> offset, string ch, int z){
    
    if (z == -1){
        z = m_index;
    }
    c_surface* surf = m_surfaces.add(size, ch, z, offset);
    m_index ++;
    return Surface(surf);
}
Tui::Tui():m_screen(m_screenDriver), m_evetBus(m_inputDriver, m_surfaces), m_screenDriver(new drivers::d_defualt_screen), m_inputDriver(new drivers::d_default_input){
    logger::info("[API] Starting up");
}
Tui::~Tui(){
    delete m_screenDriver;
    delete m_inputDriver;
}


void Tui::update(int waitinput_ms){
    static bool isDirty;
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
    m_evetBus.tick(waitinput_ms);
}

array<size_t, 2> Tui::getScreenSize(){
    return m_screen.getSize();
}

int Tui::charactersRendered(){
    return m_screen.charactersRenderd;
}
void Tui::Clock::tick() {
        auto now = clock::now();
        duration delta = now - last_frame_time;
        if (delta < target_delta) {
            std::this_thread::sleep_for(target_delta - delta * 1000);
        } 
        now = clock::now();
        delta = now - last_frame_time;
        double current_fps = 1.0 / delta.count();

        last_frame_time = now;
        if (first_frame) {
            smoothed_fps = current_fps;
            first_frame = false;
        } else {
            // The EWMA Magic
            smoothed_fps = (alpha * current_fps) + ((1.0 - alpha) * smoothed_fps);
        }
}
double Tui::Clock::get_fps() const {
    return smoothed_fps;
}
Tui::Clock::Clock(double target_fps, double smoothing_alpha) : last_frame_time(clock::now()), target_delta(1 / target_fps), alpha(smoothing_alpha){}
