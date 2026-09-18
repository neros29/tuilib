#pragma once
#include "core/tui.h"

class Surface{
private:
    // InternalSurface& m_is;
    InternalSurface* m_is;
    bool m_del = false;
public:
    Surface(InternalSurface& is);
    Surface(std::array<int, 2> size, std::string ch, int z, std::array<int, 2> offset);
    ~Surface();
    Surface();
    Surface(Surface&& surf);
    Surface& operator=(Surface&&);

    Surface(const Surface& surf) = delete;
    Surface& operator=(const Surface&) = delete;
    Character& operator[](int x);
    Character operator[](int x) const;

    bool get_event(std::string event);
    void register_keys(std::vector<std::string> keys);

    void fill_bg(int r, int g, int b);
    void fill_fg(int r, int g, int b);

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
    std::deque<InternalSurface> m_surfaces;
    std::vector<int> m_sortedSurfaceIndex;

    bool m_isSurfaceSorted;
    void m_sortSurfaces();

    int m_index;
    Screen m_screen;
    Input m_input;
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

