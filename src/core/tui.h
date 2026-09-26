#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <array>
#include <termios.h>
#include <term.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <iostream>

class InputString{
private:
    int get_byte_len(unsigned char firstbyte);
    std::vector<std::string> str;
public:
    size_t size();
    static int getWidth(std::string ch); 
    InputString(std::string s);
    std::string operator[](int i);
};
namespace drivers {
    class d_screen;
}
enum class styles : uint32_t {
    None =          0,
    BOLD =          1 << 0,
    DIM =           1 << 1,
    ITALIC =        1 << 2,
    UNDERLINE =     1 << 3,
    BLINKING =      1 << 4,
    INVERSE =       1 << 5,
    HIDDEN =        1 << 6,
    STRIKETHROUGH = 1 << 7,
};

inline styles operator|(styles lhs, styles rhs) {
    return static_cast<styles>(
        static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
    );
}

// We also overload the bitwise AND operator for checking flags
inline bool operator&(styles lhs, styles rhs) {
    return static_cast<bool>(
        static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
    );
}

namespace core{
    struct c_cell {
    private:
        uint32_t m_fg {0};
        uint32_t m_bg {0};

        char ch[5] = " ";

        styles m_styles = styles::None;
        bool ch_def {true};
        bool fg_def {true};
        bool bg_def {true};

    public:

        // setFg
        void set_fg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);

        // setBg
        void set_bg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
        void set_ch(std::string chi);
        void set_style(styles f);
        styles get_style() const;

        bool is_def(std::string value) const;
        //set ch
        
        std::string get_ch() const;
        std::array<uint8_t, 4> get_fg() const;
        std::array<uint8_t, 4> get_bg() const;

        // operators
        bool operator == (const c_cell ch) const;
        bool operator != (const c_cell ch) const;
    };

    class c_surface {
    public:
        std::vector<c_cell> surface;
        std::vector<c_cell> m_old;

        int z{0};
        bool m_dirty = true;
        std::array<int, 2> size;
        std::array<int, 2> r_cords;
        std::string defulat_ch = {" "};

        const std::vector<std::string>* m_validKeys;
        std::unordered_map<std::string, bool> events;
        std::vector<std::string> keys;

        c_surface(std::array<int, 2> size, std::string ch, int z, std::array<int, 2> offset, const std::vector<std::string>& keys);
        c_surface(std::array<int, 2> size, std::string ch, int z, std::array<int, 2> offset);
        c_cell& operator[](int x);
        c_cell operator[](int x) const;
        bool cheakDirty();
    };


    class Screen {
    private:
        drivers::d_screen* m_driver;

        const std::deque<c_surface>& m_surfaces;
        const std::vector<int>& m_sortIndex;

        std::vector<c_cell> m_screen;
        std::vector<c_cell> m_lastScreen;

        std::array<int, 2> m_size;
        std::string m_defaultCh {" "};

        void m_initScreen();
        void m_render();

    public:
        int charactersRenderd;
        Screen(const std::deque<c_surface>& surfaces, const std::vector<int>& sortIndex, drivers::d_screen* driver);
        std::array<int, 2> getSize();
        void flip();
    };

    // Input
    class Input{
    private:
        // Surface data passed from api 
        std::deque<c_surface>& m_surfaces; 
        const std::vector<int>& m_sortIndex;

        // input stuff
        struct termios m_old;
        std::vector<std::array<std::string, 2>> m_namePair;
        std::unordered_map<std::string, std::string> m_termDb;

        // input buffer
        std::string m_buffer;

        // member private functions
        inline void m_initNamePair();
        void m_setRawMode();
        void m_restoreMode();
        void m_initDb();
        void m_add(std::string capname, std::string human);
        std::string m_getCap(const char* name);
    public:
        std::vector<std::string> keys;
        // Constructers
        Input(std::deque<c_surface>& surfaces, const std::vector<int>& sortIndex);
        ~Input();

        // member public functions
        bool update();
    };

}
