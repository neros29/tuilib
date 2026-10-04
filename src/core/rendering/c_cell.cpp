#include <array>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <string>
#include <variant>
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
}
void c_cell::set_bg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha){
    m_setColor(m_bg, {red, green, blue, alpha});
}
void c_cell::set_ch(std::string chi){
    if (chi.size() < 8){
        array<char, 8> data;
        for (int i = 0; i < 8; i ++){
            if (i < chi.size()){
                data[i] = chi[i];
            } else {
                data[i] = '\0';
            }
        }
        m_ch = data;
    } else {
        m_ch = new std::string(chi);
    }
}
std::string c_cell::get_ch() const{
    if (holds_alternative<std::monostate>(m_ch)){
        return " ";
    }
    else if (auto v = std::get_if<std::array<char, 8>>(&m_ch)){
        return std::string(v->data());
    }
    return std::string(*std::get<std::string*>(m_ch));
}
std::array<uint8_t, 4> c_cell::get_fg() const{
    return m_getColor(m_fg);
}
std::array<uint8_t, 4> c_cell::get_bg() const{
    return m_getColor(m_bg);
}
void c_cell::set_style(uint16_t flags){
    m_flags = (m_flags & ~flags::STYLE_MASK) | ((static_cast<uint32_t>(flags) << flags::STYLE_SHIFT) & flags::STYLE_MASK);
}
void c_cell::or_style(uint16_t flags){
    m_flags = m_flags | ((static_cast<uint32_t>(flags) << flags::STYLE_SHIFT) & flags::STYLE_MASK);
}
uint16_t c_cell::get_style() const{
    return static_cast<uint16_t>((m_flags & flags::STYLE_MASK) >> flags::STYLE_SHIFT);
}
void c_cell::set_flags(uint16_t flags){
    m_flags = (m_flags & ~flags::STYLE_MASK) | (static_cast<uint32_t>(flags) << flags::STYLE_SHIFT);
};
void c_cell::or_flags(uint16_t flags){
    m_flags = m_flags | (static_cast<uint32_t>(flags) << flags::SYSTEM_SHIFT);
}
uint16_t c_cell::get_flags() const{
    return static_cast<uint16_t>((m_flags & flags::SYSTEM_MASK) >> flags::SYSTEM_SHIFT);
}
bool c_cell::is_def(string type) const{
    if (type == "ch"){
        return std::get_if<std::monostate>(&m_ch);
    }
    else if (type == "fg"){
        return !static_cast<bool>(m_fg);
    }
    else if (type == "bg"){
        return !static_cast<bool>(m_bg);
    }
    else {
        throw invalid_argument{"You must pass eather ch, fg, or bg into get_def."};
    }
}
bool c_cell::operator ==(const c_cell& chi) const{
    if (chi.m_bg != m_bg){
        return false;
    }
    if (chi.m_fg != m_fg){
        return false;
    }
    if (chi.m_ch != m_ch){
        return false;
    }
    if (chi.m_flags != m_flags){
        return false;
    }
    return true;
}

bool c_cell::operator !=(const c_cell& chi) const{
    return  !c_cell::operator==(chi);
}
void c_cell::clear(){
    if (holds_alternative<std::string*>(m_ch)){
        delete std::get<std::string*>(m_ch);
    }
    m_fg = 0;
    m_bg = 0;
    m_ch = std::monostate();
    m_flags = styles::NONE;
}
