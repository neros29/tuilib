#pragma once
#include "tui.h"
#include <array>
#include <cstddef>
#include <inttypes.h>
#include <vector>
#include <termios.h>

namespace drivers {
    class d_screen{
    public:
        virtual ~d_screen(){};
        virtual void d_draw(int x, int y, const core::c_cell& cell) = 0;
        virtual void d_startFrame() = 0;
        virtual void d_endFrame() = 0;
        virtual std::array<size_t, 2> d_getWinSize() = 0;
    };
    class d_input{
    public:
        virtual ~d_input(){};
        virtual std::vector<Event> d_getEvents(int timeout_ms) = 0;
        virtual void d_clean() = 0;
        // virtual int d_getCharacterWidth(uint32_t ch) = 0;
    };
}
