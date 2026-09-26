#include <stdexcept>
#include <string>
#include "../tui.h"
#include "../utils/logging.h"

using namespace std;
using namespace core;
using namespace drivers;


static void cheakInput(int r, int g, int b, int a){
    if (r > 255 || g > 255 || b > 255 || a > 255){
        throw invalid_argument{"colors must be bettween 0 and 255"};
    }
    if (r < 0 || g < 0 || b < 0 || a < 0){
        throw invalid_argument{"colors must be none negative"};
    }
}
static std::array<uint8_t, 4> m_getColor(const uint32_t color) {
    std::array<uint8_t, 4> arr;
    for (int i = 0; i < arr.size(); i++){
        arr[i] = (color >> (i*8)) & 0xff;
    }
    return arr;
}
static void m_clear(uint32_t& color){
    color = 0;
}

static void m_setColor(uint32_t& color, std::array<uint8_t, 4> arr){
    m_clear(color);
    for (int i = 0; i < arr.size(); i++){
        color |= static_cast<uint32_t>(arr[i]) << (8 * i); 
    }
}
void c_cell::set_fg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
    m_setColor(m_fg, {red, green, blue, alpha});
    fg_def = false;
}
void c_cell::set_bg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
    m_setColor(m_bg, {red, green, blue, alpha});
    bg_def = false;
}
void c_cell::set_ch(std::string chi){
    int index = 0;
    for (int i = 0; i < 5; i++){
        if (i < chi.size()){
            ch[i] = chi[i];
        } else {
            ch[i] = '\0';
        }
    }
    ch_def = false;
}
std::string c_cell::get_ch() const{
    return std::string {ch};
}
std::array<uint8_t, 4> c_cell::get_fg() const{
    return m_getColor(m_fg);
}
std::array<uint8_t, 4> c_cell::get_bg() const{
    return m_getColor(m_bg);
}
void c_cell::set_style(styles f){
    m_styles = f;
}
styles c_cell::get_style() const{
    return m_styles;
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
