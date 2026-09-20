#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <array>
#include <termios.h>
#include <term.h>

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

namespace core {

    struct Character {
    private:
        // Initialize to invalid color to make sure the == operator works correctly.
        std::array<int, 4> fg {0, 0, 0, 0};
        std::array<int, 4> bg {255, 0, 0, 0};
        std::string ch = " ";
        bool ch_def {true};
        bool fg_def {true};
        bool bg_def {true};

    public:


        void set_fg(int r, int g, int b, int a = 255);
        void set_bg(int r, int g, int b, int a = 255);
        void set_ch(std::string chi);

        bool is_def(std::string value) const;

        std::array<int, 4> get_fg() const;
        std::array<int, 4> get_bg() const;
        std::string get_ch() const;

        bool operator == (const Character ch) const;
        bool operator != (const Character ch) const;
    };

    class c_surface {
    public:
        std::vector<Character> surface;
        std::vector<Character> m_old;

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
        Character& operator[](int x);
        Character operator[](int x) const;
        bool cheakDirty();
    };



    class Screen {
    private:
        const std::deque<c_surface>& m_surfaces;
        std::vector<Character> m_screen;
        std::vector<Character> m_lastScreen;
        const std::vector<int>& m_sortIndex;
        std::array<int, 2> m_size;
        std::ofstream m_file;
        int m_index{0};
        std::string m_defaultCharacter {" "};
        bool m_sizeChange {true};
        bool m_sorted {false};

        void m_getWinSize();
        void m_initScreen();
        std::string m_genrate(Character& ch);
        void m_render();

    public:
        int charactersRenderd;
        Screen(const std::deque<c_surface>& surfaces, const std::vector<int>& sortIndex);
        ~Screen();
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
