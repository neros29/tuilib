#pragma once
#include "core/tui.h"

class Surface{
private:
    // InternalSurface& m_is;
    InternalSurface* m_is;
    bool m_del = false;
public:
    Surface(InternalSurface& is);
    Surface(array<int, 2> size, string ch, int z, array<int, 2> offset);
    ~Surface();
    Surface();
    Surface(Surface&& surf);
    Surface& operator=(Surface&&);
    Character& operator[](int x);
    Character operator[](int x) const;

    bool get_event(string event);
    void register_keys(vector<string> keys);

    void fill_bg(int r, int g, int b);
    void fill_fg(int r, int g, int b);

    void set_z(int z);
    int get_z() const;


    void set_offset(int x, int y);
    array<int, 2> get_offset() const;

    array<int, 2> get_size() const; 
    void blit(Surface& surf);
};

class Tui{
private:
    deque<InternalSurface> m_surfaces;
    vector<int> m_sortedSurfaceIndex;

    bool m_isSurfaceSorted;
    void m_sortSurfaces();

    int m_index;
    Screen m_screen;
    Input m_input;
    ofstream m_logFile; 
public:
    int skipedFrames = 0;
    Tui();
    ~Tui();
    Surface append(array<int, 2> size, array<int, 2> offset, string ch = " ", int z = -1);
    void update();
    array<int, 2> getScreenSize();
    int charactersRendered();

};

