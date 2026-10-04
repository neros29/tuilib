#include "../api.h"
#include "../core/tui.h"
#include "../core/utils/logging.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath>

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
Surface::Surface(c_surface* is) : m_is(is), id(m_is->id) {}
Surface::Surface(array<int, 2> size, string ch, int z, array<int, 2> offset)
    : m_is(new c_surface(size, ch, z, offset)) {
        m_del = true;
    }
Surface::Surface() : m_is(nullptr) {}
Surface::Surface(Surface &&surf) {
    m_is = surf.m_is;
    m_del = surf.m_del;
    surf.m_is = nullptr;
}
Surface::~Surface() {
    if (m_del) {
        assert(m_is != nullptr);
        delete m_is;
    }
}
Surface &Surface::operator=(Surface &&surf) {
    if (this != &surf) {
        delete m_is;         // free old resource
        m_is = surf.m_is;    // steal pointer
        m_del = surf.m_del;  // take ownership flag
        surf.m_is = nullptr; // leave moved-from in safe state
        surf.m_del = false;
    }
    return *this;
}

void Surface::fill_bg(int r, int g, int b, int a) {
    assert(m_is != nullptr);
    for (int i = 0; i != m_is->surface.size(); i++) {
        m_is->surface[i].set_bg(r, g, b, a);
    }
}
void Surface::fill_fg(int r, int g, int b, int a) {
    assert(m_is != nullptr);
    for (int i = 0; i != m_is->surface.size(); i++) {
        m_is->surface[i].set_fg(r, g, b, a);
    }
}
void Surface::fill_style(uint32_t s) {
    assert(m_is != nullptr);
    for (int i = 0; i != m_is->surface.size(); i++) {
        m_is->surface[i].set_style(s);
    }
}

bool Surface::get_event(string event) {
    assert(m_is != nullptr);
    if (bool isEvent = m_is->events[event]) {
        m_is->events[event] = false;
        return isEvent;
    }
    return false;
}
void Surface::register_keys(vector<string> i_keys) {
    assert(m_is != nullptr);
    for (string key : i_keys) {
        if (find(m_is->m_validKeys->begin(), m_is->m_validKeys->end(), key) ==
                m_is->m_validKeys->end() &&
                key.size() > 1) {
            throw invalid_argument{"The key " + key +
                " dose not exist in input's data base"};
        }
        m_is->keys.push_back(key);
    }
}

void Surface::set_z(int zi) {
    assert(m_is != nullptr);
    if (zi != m_is->z) {
        m_is->m_dirty = true;
        m_is->z = zi;
    }
}
int Surface::get_z() const {
    assert(m_is != nullptr);
    return m_is->z;
}

array<int, 2> Surface::get_size() const {
    assert(m_is != nullptr);
    return m_is->size;
}
array<int, 2> Surface::get_offset() const {
    assert(m_is != nullptr);
    return m_is->r_cords;
}
void Surface::set_offset(int x, int y) {
    if (x != m_is->r_cords[0] || y != m_is->r_cords[1]) {
        m_is->m_dirty = true;
        m_is->r_cords[0] = x;
        m_is->r_cords[1] = y;
    }
}

void Surface::extend_size(string direction, int amount) {
    c_cell chr;
    chr.set_ch(m_is->defulat_ch);
    if (direction == "Down"){
        m_is->size[1] += amount;
        if (amount > 0){
            m_is->surface.resize(m_is->size[0] * m_is->size[1], chr);
        } else {
            int ammont_to_erase = std::abs(amount);
            if (ammont_to_erase > m_is->size[1]){
                throw invalid_argument("Size must not be zero.");
            } else {
                m_is->surface.erase(m_is->surface.begin(), m_is->surface.begin() + ammont_to_erase * m_is->size[0]);
            }
        }
    }
    if (direction == "Up"){
        m_is->size[1] += amount;
        if (amount > 0){
            m_is->surface.insert(m_is->surface.begin(), amount * m_is->size[0], chr);
        } else {
            int ammont_to_erase = std::abs(amount);
            if (ammont_to_erase > m_is->size[1]){
                throw invalid_argument("Size must not be zero.");
            } else {
                m_is->surface.erase(m_is->surface.end() - ammont_to_erase * m_is->size[0], m_is->surface.end());
            }
        }
    }
    if (direction == "Right" || direction == "Left"){
        int left_padding = 0;
        int smaller = amount;
        if (direction == "Left"){
            left_padding = amount;
        }
        if (amount < 0){
            if (std::abs(amount) > m_is->size[1]){
                throw invalid_argument("Size must not be zero.");
            }
            smaller = 0;
        }
        m_is->size[0] += amount;
        vector<c_cell> new_vec(m_is->size[0] * m_is->size[1], chr);
        for (int y = 0; y < m_is->size[1] - smaller; y++){
            for (int x = 0; x < m_is->size[0] - smaller; x++){
                int old_idx = y * (m_is->size[0] - amount) + x;
                c_cell& ch = m_is->surface[old_idx];
                if (ch != chr){
                    int new_idx = y * m_is->size[0] + x + left_padding;
                    new_vec[new_idx] = ch;
                }
            }
        }
        m_is->surface.swap(new_vec);
    }
    m_is->m_dirty = true;
}

void Surface::blit(Surface &surf) {
    assert(m_is->surface.size() == m_is->size[0] * m_is->size[1]);
    int x = 0;
    int y = 0;
    for (int i = 0; i != m_is->surface.size(); i++) {
        bool def{true};
        int r_x = x - surf.m_is->r_cords[0];
        int r_y = y - surf.m_is->r_cords[1];
        if (r_x < surf.m_is->size[0] && r_y < surf.m_is->size[1]) {
            if (r_x >= 0 && r_y >= 0) {
                int index = (surf.m_is->size[0] * r_y) + r_x;
                c_cell& newCharacter = surf.m_is->surface[index];
                c_cell& oldCharacter = m_is->surface[i];

                if (!newCharacter.is_def("ch")){
                    oldCharacter.set_ch(newCharacter.get_ch());
                    oldCharacter.or_style(newCharacter.get_style());
                    if (!newCharacter.is_def("fg")){
                        auto fg = newCharacter.get_fg();
                        oldCharacter.set_fg(fg[0], fg[1], fg[2], fg[3]);
                    }
                } 
                oldCharacter.or_flags(newCharacter.get_flags());
                auto bg = blendColors(oldCharacter.get_bg(), newCharacter.get_bg());
                oldCharacter.set_bg(bg[0], bg[1], bg[2], bg[3]);
            }
        }
        if (x == m_is->size[0] - 1) {
            x = 0;
            y++;
        } else {
            x++;
        }
    }
    m_is->m_dirty = true;
}
Cell Surface::operator[](int x) {return Cell(*m_is, m_is->surface[x]); }
const Cell Surface::operator[](int x) const { return Cell(*m_is, m_is->surface[x]); }
