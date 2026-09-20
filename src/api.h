#pragma once
#include "core/tui.h"

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
    core::Character& operator[](int x);
    core::Character operator[](int x) const;

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

