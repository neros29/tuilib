#include <array>
#include <cassert>
#include <string>
#include <sys/types.h>
#include <vector>
#include <sys/ioctl.h>
#include <cmath> // for std::round

#include "../tui.h"
#include "../utils/logging.h"
#include "../drivers.h"

using namespace std;
using namespace core;
/**
 * Blends two RGBA color arrays.
 * @param src The foreground color array [R, G, B, A] (0-255)
 * @param dst The background color array [R, G, B, A] (0-255)
 * @return A new 4-element array containing the blended color.
 */
static array<u_int8_t, 4> blendColors(const std::array<u_int8_t, 4>& src, const std::array<u_int8_t, 4>& dst) {
    if (src[3] <= 0){
        return dst;
    }
    else if (dst[3] <= 0){
        return src;
    }
    array<u_int8_t, 4> result;

    // Convert alpha to 0.0 - 1.0 range for math
    float alpha_src = src[3] / 255.0f;
    float alpha_dst = dst[3] / 255.0f;
    float inv_alpha_src = 1.0f - alpha_src;

    // 1. Blend RGB channels
    // Formula: C_out = (C_src * alpha_src) + (C_dst * (1 - alpha_src))
    result[0] = static_cast<int>(round(src[0] * alpha_src + dst[0] * inv_alpha_src));
    result[1] = static_cast<int>(round(src[1] * alpha_src + dst[1] * inv_alpha_src));
    result[2] = static_cast<int>(round(src[2] * alpha_src + dst[2] * inv_alpha_src));

    // 2. Accumulate the Alpha channel
    // Formula: A_out = A_src + A_dst * (1 - A_src)
    float alpha_out = alpha_src + (alpha_dst * inv_alpha_src);
    result[3] = static_cast<int>(round(alpha_out * 255.0f));

    return result;
}


array<int, 2> Screen::getSize(){
    return m_size;
}

void Screen::m_initScreen(){
    c_cell cell;
    cell.set_ch(m_defaultCh);
    m_screen.assign(m_size[0] * m_size[1], cell);
    m_lastScreen.assign(m_size[0] * m_size[1], cell);
}

void Screen::m_render(c_surfaces& surfs){
    array<int, 2> new_size = m_driver->d_getWinSize();
    if (new_size != m_size){
        m_size = new_size;
        m_initScreen();
    }
    array<u_int8_t, 4> bg {0, 0, 0, 0}; // pre make these so we aren't allocating and destroying for every single cell. Probably not necessary.
    array<u_int8_t, 4> fg {0, 0, 0, 0};
    for (int i = 0; i < m_screen.size(); i++){
        int y = i / m_size[0];
        int x = i % m_size[0];
        c_cell& newCell = m_screen[i];
        newCell.clear();
        for (c_surface& surf: surfs){ // m_sortIndex is from highest z to lowest z
            int r_x = x - surf.r_cords[0];
            int r_y = y - surf.r_cords[1];
            if (r_x < surf.size[0] && r_y < surf.size[1]){
                if (r_x >= 0 && r_y >= 0){
                    int index = surf.size[0] * r_y + r_x;
                    const c_cell currentCell = surf[index];

                    if (!currentCell.is_def("ch") && newCell.is_def("ch") && currentCell.get_ch() != m_defaultCh){
                        newCell.set_ch(currentCell.get_ch());
                        fg = blendColors(newCell.get_fg(), currentCell.get_fg());
                        newCell.set_style(currentCell.get_style());
                        newCell.set_fg(fg[0], fg[1], fg[2], fg[3]);
                    } 
                    newCell.or_flags(currentCell.get_flags());
                    bg = blendColors(newCell.get_bg(), currentCell.get_bg());
                    newCell.set_bg(bg[0], bg[1], bg[2], bg[3]);

                    fg = blendColors(newCell.get_fg(), currentCell.get_bg());
                    newCell.set_fg(fg[0], fg[1], fg[2], fg[3]);

                    if (bg[3] >= 255){
                        break;
                    }
                }
            }
        }
    }
}


void Screen::flip(c_surfaces& surfs){
    m_render(surfs);
    m_driver->d_startFrame();
    for (int i = 0; i < m_screen.size(); i++){
        int y = i / m_size[0] + 1;
        int x = i % m_size[0] + 1; // add one to both x, and y since terminals are 1 indexed.
        if (m_screen[i].get_flags() & core::flags::PLACEHOLDER){ // Don't render anything if it's a place holder cell.
            continue;
        }
        if (m_screen[i] != m_lastScreen[i]){
            m_driver->d_draw(x, y, m_screen[i]);
            charactersRenderd++;
        } 
    }
    m_lastScreen.swap(m_screen);
    m_driver->d_endFrame();
}
Screen::Screen(drivers::d_screen* driver): m_driver(driver){
    logger::info("[Screen] Starting up");
    m_size = m_driver->d_getWinSize();
    m_initScreen();
}

