#pragma once
#include "core/tui.h"
#include <array>

class Cell{
    core::c_surface& surf;
    core::c_cell& cell;
    public:

    Cell(core::c_surface& c_surface, core::c_cell& c_cell): surf(c_surface), cell(c_cell){}

    void set_fg(std::array<uint8_t, 4> color){
        cell.set_bg(color[0], color[1], color[2], color[3]);
        surf.m_dirty = true;
    }
    void set_fg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255){
        cell.set_fg(red, green, blue, alpha);
        surf.m_dirty = true;
    }
    void set_bg(std::array<uint8_t, 4> color){
        cell.set_bg(color[0], color[1], color[2], color[3]);
        surf.m_dirty = true;
    }
    void set_bg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255){
        cell.set_bg(red, green, blue, alpha);
        surf.m_dirty = true;
    }
    void set_ch(std::string chi){
        cell.set_ch(chi);
        surf.m_dirty = true;
    }
    std::string get_ch() const{
        return cell.get_ch();
    }
    std::array<uint8_t, 4> get_fg() const{
        return cell.get_fg();
    }
    std::array<uint8_t, 4> get_bg() const{
        return cell.get_fg();
    }
};

class Surface{
private:
    // c_surface& m_is;
    core::c_surface* m_is;
    bool m_del = false;
public:
    Surface(core::c_surface& is);
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
    std::deque<core::c_surface> m_surfaces;
    std::vector<int> m_sortedSurfaceIndex;

    bool m_isSurfaceSorted;
    void m_sortSurfaces();

    int m_index;
    core::Screen m_screen;
    core::Input m_input;
    std::ofstream m_logFile; 
public:
    int skipedFrames = 0;
    Tui();
    int amontOfSurfaces(){
        return m_surfaces.size();
    }
    int amontOfCharacters(){
        return m_screen.getSize()[0] * m_screen.getSize()[1];
    }
    Surface append(std::array<int, 2> size, std::array<int, 2> offset, std::string ch = " ", int z = -1);
    void update();
    std::array<int, 2> getScreenSize();
    int charactersRendered();

};

