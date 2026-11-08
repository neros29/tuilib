#pragma once
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <array>
#include <termios.h>
#include <term.h>
#include <inttypes.h>
#include <sys/ioctl.h>

class InputString{
private:
    std::vector<std::string> str;
public:
    static int get_byte_len(unsigned char firstbyte);
    size_t size();
    static int getWidth(std::string ch); 
    InputString(std::string s);
    std::string operator[](int i);
};
namespace drivers {
    class d_screen;
    class d_input;
}
namespace styles{
    constexpr uint16_t NONE =          0;
    constexpr uint16_t BOLD =          1 << 0;
    constexpr uint16_t DIM =           1 << 1;
    constexpr uint16_t ITALIC =        1 << 2;
    constexpr uint16_t UNDERLINE =     1 << 3;
    constexpr uint16_t BLINKING =      1 << 4;
    constexpr uint16_t INVERSE =       1 << 5;
    constexpr uint16_t HIDDEN =        1 << 6;
    constexpr uint16_t STRIKETHROUGH = 1 << 7;
};
enum class modifers : uint8_t{
    none =      0,
	shift =     0b1,
	alt =       0b10,
	ctrl =      0b100,
	super =     0b1000,
	hyper =     0b10000,
	meta =      0b100000,
	caps_lock = 0b1000000,
	num_lock =  0b10000000,
};
inline modifers operator|(modifers lhs, modifers rhs) {
    return static_cast<modifers>(
        static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
    );
}

// We also overload the bitwise AND operator for checking flags
inline bool operator&(modifers lhs, modifers rhs) {
    return static_cast<bool>(
        static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs)
    );
}
enum class Key : uint32_t{
    NONE,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    Delete,
    Insert,
    PageUp,
    PageDown,
    Backspace,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
};
struct Event{
    enum class Kind { Key, Text } kind;
    modifers modifer;
    std::string text;
    Key key;
};
namespace core{
    namespace flags {
        constexpr uint16_t PLACEHOLDER = 1 << 0;

        constexpr uint32_t STYLE_MASK = 0x0000FFFF; // Bits 0-15 are 1, others are 0
        constexpr uint32_t SYSTEM_MASK = 0xFFFF0000; // Bits 16-31 are 1, others are 0
        // The Shifts: used to move values into their designated slots
        constexpr int STYLE_SHIFT = 0;
        constexpr int SYSTEM_SHIFT = 16;
    }
    struct c_cell {
        uint32_t m_fg {0};
        uint32_t m_bg {0};
        std::variant<std::monostate, std::array<char, 8>, std::string*> m_ch;
        uint32_t m_flags = styles::NONE;
    public:
        // setFg
        void set_fg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);

        // setBg
        void set_bg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
        void set_ch(std::string chi);
        void set_style(uint16_t flag);
        void or_style(uint16_t flag);
        uint16_t get_style() const;

        void set_flags(uint16_t flag);
        void or_flags(uint16_t flag);
        uint16_t get_flags() const;

        bool is_def(std::string value) const;
        //set ch
        
        std::string get_ch() const;
        std::array<uint8_t, 4> get_fg() const;
        std::array<uint8_t, 4> get_bg() const;
        void clear();

        // operators
        bool operator == (const c_cell& ch) const;
        bool operator != (const c_cell& ch) const;
    };
    class c_surface {
    public:
        int id;
        std::vector<c_cell> surface;
        std::vector<c_cell> m_old;
        std::function<bool(Event)> inputFunc {nullptr};
        int z{0};
        bool m_dirty = true;
        std::array<size_t, 2> size;
        std::array<int, 2> r_cords;
        std::string defulat_ch = {" "};

        std::unordered_map<std::string, bool> events;
        std::vector<std::string> keys;

        c_surface(std::array<size_t, 2> size, std::string ch, int z, std::array<int, 2> offset, int id);
        c_surface(std::array<size_t, 2> size, std::string ch, int z, std::array<int, 2> offset);
        c_cell& operator[](size_t x);
        const c_cell& operator[](size_t x) const;
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
        c_surface* add(std::array<size_t, 2> size, std::string ch, int z, std::array<int, 2> offset);
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

        std::array<size_t, 2> m_size;
        std::string m_defaultCh {" "};

        void m_initScreen();
        void m_render(c_surfaces& surfs);

    public:
        int charactersRenderd;
        Screen(drivers::d_screen* driver);
        std::array<size_t, 2> getSize();
        void flip(c_surfaces& surfs);
    };

    // Input
    class Input{
    private:
        // input stuff
        drivers::d_input* m_driver;

    public:
        // Constructers
        Input(drivers::d_input* driver);
        ~Input();

        // member public functions
        bool update(c_surfaces& surfs, int timeout_ms = -1);
    };
    class c_eventBus{
    private:
        drivers::d_input* m_inputDriver;
        c_surfaces& m_surfs;
    public:
        c_eventBus(drivers::d_input* driver, c_surfaces& surfs);
        ~c_eventBus();
        void tick(int timeout_ms);
    };
}
