#pragma once
#include "core/tui.h"

class Tui{
private:
    deque<Surface> m_surfaces;
    vector<int> m_sortedSurfaceIndex;

    bool m_isSurfaceSorted;
    void m_sortSurfaces();

    Screen m_screen;
    Input m_input;
    ofstream m_logFile; 
public:
    int skipedFrames = 0;
    Tui();
    ~Tui();
    Surface& append(array<int, 2> size, array<int, 2> offset, string ch = " ", int z = -1);
    void update();
    array<int, 2> getScreenSize();
    int charactersRendered();

};

