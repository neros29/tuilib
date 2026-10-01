#pragma once
#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <termios.h>
#include <term.h>
#include <inttypes.h>
#include <sys/ioctl.h>

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
        void clear();

        // operators
        bool operator == (const c_cell ch) const;
        bool operator != (const c_cell ch) const;
    };
    class c_surface {
    public:
        int id;
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

        c_surface(std::array<int, 2> size, std::string ch, int z, std::array<int, 2> offset, const std::vector<std::string>& keys, int id);
        c_surface(std::array<int, 2> size, std::string ch, int z, std::array<int, 2> offset);
        c_cell& operator[](int x);
        c_cell operator[](int x) const;
        bool operator==(const c_surface& surf) const;
        bool cheakDirty();
    };

    class c_surfaces {
    private:
        std::deque<core::c_surface> m_surfaces;
        std::vector<c_surface*> m_sortedSurfaces;
        int m_nextId {0};
    public:
         class SurfaceIterator {
            public:
                using iterator_category = std::forward_iterator_tag;
                using value_type = c_surface&;
                using difference_type = std::ptrdiff_t;
                using pointer = c_surface**;
                using reference = c_surface&;

                SurfaceIterator(std::vector<c_surface*>::iterator it) : m_it(it) {}

                reference operator*() const { return *(*m_it); }
                c_surface* operator->() const { return *m_it; }

                SurfaceIterator& operator++() { ++m_it; return *this; }
                SurfaceIterator operator++(int) { SurfaceIterator tmp = *this; ++(*this); return tmp; }
                bool operator==(const SurfaceIterator& other) const { return m_it == other.m_it; }
                bool operator!=(const SurfaceIterator& other) const { return m_it != other.m_it; }

            private:
                std::vector<c_surface*>::iterator m_it;
            };
        c_surface* add(std::array<int, 2> size, std::string ch, int z, std::array<int, 2> offset, const std::vector<std::string>& keys);
        void sort();
        size_t size();
        SurfaceIterator begin();
        SurfaceIterator end();
    };

    class Screen {
    private:
        drivers::d_screen* m_driver;
        std::vector<c_cell> m_screen;
        std::vector<c_cell> m_lastScreen;

        std::array<int, 2> m_size;
        std::string m_defaultCh {" "};

        void m_initScreen();
        void m_render(c_surfaces& surfs);

    public:
        int charactersRenderd;
        Screen(drivers::d_screen* driver);
        std::array<int, 2> getSize();
        void flip(c_surfaces& surfs);
    };

    // Input
    class Input{
    private:
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
        Input();
        ~Input();

        // member public functions
        bool update(c_surfaces& surfs);
    };
}
