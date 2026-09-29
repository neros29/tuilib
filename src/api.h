#pragma once
#include "core/tui.h"
#include <array>
#include <cstdint>

class Cell{
    core::c_surface& m_surf;
    core::c_cell& m_cell;
    public:

    Cell(core::c_surface& c_surface, core::c_cell& c_cell): m_surf(c_surface), m_cell(c_cell){}

    Cell(const Cell&) = delete;
    Cell& operator=(const Cell&) = delete;

    Cell(const Cell&& cell): m_surf(cell.m_surf), m_cell(cell.m_cell){}
    Cell& operator=(const Cell&& cell){
        m_cell = cell.m_cell;
        m_surf = cell.m_surf;
        return *this;
    }

    void set_fg(std::array<uint8_t, 4> color){
        m_cell.set_fg(color[0], color[1], color[2], color[3]);
        m_surf.m_dirty = true;
    }
    void set_fg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255){
        m_cell.set_fg(red, green, blue, alpha);
        m_surf.m_dirty = true;
    }
    void set_bg(std::array<uint8_t, 4> color){
        m_cell.set_bg(color[0], color[1], color[2], color[3]);
        m_surf.m_dirty = true;
    }
    void set_bg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255){
        m_cell.set_bg(red, green, blue, alpha);
        m_surf.m_dirty = true;
    }
    void set_ch(std::string chi){
        m_cell.set_ch(chi);
        m_surf.m_dirty = true;
    }
    void set_style(styles style){
        m_cell.set_style(style);
    }
    std::string get_ch() const{
        return m_cell.get_ch();
    }
    std::array<uint8_t, 4> get_fg() const{
        return m_cell.get_fg();
    }
    std::array<uint8_t, 4> get_bg() const{
        return m_cell.get_fg();
    }
    styles get_style(){
        return m_cell.get_style();
    }
};

class Surface{
private:
    core::c_surface* m_is;
    bool m_del = false;
public:
    int id;
    Surface(core::c_surface* is);
    Surface(std::array<int, 2> size, std::string ch, int z, std::array<int, 2> offset);
    ~Surface();
    Surface();
    Surface(Surface&& surf);
    Surface& operator=(Surface&&);

    Surface(const Surface& surf) = delete;
    Surface& operator=(const Surface&) = delete;
    Cell operator[](int x);
    Cell operator[](int x) const;

    bool get_event(std::string event);
    void register_keys(std::vector<std::string> keys);

    void fill_bg(int r, int g, int b, int a = 255);
    void fill_fg(int r, int g, int b, int a = 255);
    void fill_style(styles s);

    void set_z(int z);
    int get_z() const;

    void set_offset(int x, int y);
    std::array<int, 2> get_offset() const;
    
    void extend_size(std::string direction, int amount);

    std::array<int, 2> get_size() const; 
    void blit(Surface& surf);
};

class Tui{
private:
    drivers::d_screen* m_screenDriver;
    core::c_surfaces m_surfaces;
    bool m_isSurfaceSorted;
    void m_sortSurfaces();

    int m_index = 0;
    core::Screen m_screen;
    core::Input m_input;
public:
    int skipedFrames = 0;
    Tui();
    ~Tui();
    int amontOfSurfaces(){
        return m_surfaces.size();
    }
    int amontOfCharacters(){
        return m_screen.getSize()[0] * m_screen.getSize()[1];
    }
    Surface addSurface(std::array<int, 2> size, std::array<int, 2> offset, std::string ch = " ", int z = -1);
    void update();
    std::array<int, 2> getScreenSize();
    int charactersRendered();

};

