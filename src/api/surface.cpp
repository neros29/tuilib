#include "../api.h"
#include "../core/tui.h"
#include "../core/utils/logging.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace core;

Surface::Surface(InternalSurface &is) : m_is(&is) {}
Surface::Surface(array<int, 2> size, string ch, int z, array<int, 2> offset)
    : m_is(new InternalSurface(size, ch, z, offset)) {
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

void Surface::fill_bg(int r, int g, int b) {
  assert(m_is != nullptr);
  for (int i = 0; i != m_is->surface.size(); i++) {
    m_is->surface[i].set_bg(r, g, b);
  }
}
void Surface::fill_fg(int r, int g, int b) {
  assert(m_is != nullptr);
  for (int i = 0; i != m_is->surface.size(); i++) {
    m_is->surface[i].set_fg(r, g, b);
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
    Character chr;
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
        vector<Character> new_vec(m_is->size[0] * m_is->size[1], chr);
        for (int y = 0; y < m_is->size[1] - smaller; y++){
            for (int x = 0; x < m_is->size[0] - smaller; x++){
                int old_idx = y * (m_is->size[0] - amount) + x;
                Character& ch = m_is->surface[old_idx];
                if (ch != chr){
                    int new_idx = y * m_is->size[0] + x + left_padding;
                    new_vec[new_idx] = ch;
                }
            }
        }
        m_is->surface.swap(new_vec);
    }
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
                if (!surf[index].bg_def) {
                    m_is->surface[i].set_bg(surf[index].bg[0], surf[index].bg[1],
                            surf[index].bg[2]);
                }
                if (!surf[index].fg_def) {
                    m_is->surface[i].set_fg(surf[index].fg[0], surf[index].fg[1],
                            surf[index].fg[2]);
                }
                if (!surf[index].ch_def) {
                    m_is->surface[i].set_ch(surf[index].ch);
                }
            }
        }
        if (x == m_is->size[0] - 1) {
            x = 0;
            y++;
        } else {
            x++;
        }
    }
}

Character &Surface::operator[](int x) { return m_is->surface[x]; }
Character Surface::operator[](int x) const { return m_is->surface[x]; }
