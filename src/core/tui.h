#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <array>
#include <termios.h>
#include <term.h>
#include <cstdint>

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

    struct c_cell {
    private:
        uint32_t m_fg {0};
        uint32_t m_bg {0};

        char ch[5] = " ";

        bool ch_def {true};
        bool fg_def {true};
        bool bg_def {true};

        std::array<uint8_t, 4> m_getColor(const uint32_t color) const{
            std::array<uint8_t, 4> arr;
            for (int i = 0; i < arr.size(); i++){
                arr[i] = (color >> (i*8)) & 0xff;
            }
            return arr;
        }
        void m_clear(uint32_t& color){
            color = 0;
        }
        void m_setColor(uint32_t& color, std::array<uint8_t, 4> arr){
            m_clear(color);
            for (int i = 0; i < arr.size(); i++){
                color |= static_cast<uint32_t>(arr[i]) << (8 * i); 
            }
            }
    public:

        // setFg
        void set_fg(std::array<uint8_t, 4> color){
            m_setColor(m_fg, {color[0], color[1], color[2], color[3]});
            fg_def = false;
        }
        void set_fg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255){
            m_setColor(m_fg, {red, green, blue, alpha});
            fg_def = false;
        }

        // setBg
        void set_bg(std::array<uint8_t, 4> color){
            m_setColor(m_bg, {color[0], color[1], color[2], color[3]});
            bg_def = false;
        }
        void set_bg(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255){
            m_setColor(m_bg, {red, green, blue, alpha});
            bg_def = false;
        }
        void set_ch(std::string chi){
            int index = 0;
            for (int i = 0; i < 5; i++){
                if (i < chi.size()){
                    ch[i] = chi[i];
                } else {
                    ch[i] = '\0';
                }
            }
            ch_def = false;
        }

        bool is_def(std::string value) const;
        //set ch
        
        std::string get_ch() const{
            return std::string {ch};
        }
        std::array<uint8_t, 4> get_fg() const{
            return m_getColor(m_fg);
        }
        std::array<uint8_t, 4> get_bg() const{
            return m_getColor(m_bg);
        }

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
        const std::deque<c_surface>& m_surfaces;
        std::vector<c_cell> m_screen;
        std::vector<c_cell> m_lastScreen;
        const std::vector<int>& m_sortIndex;
        std::array<int, 2> m_size;
        std::ofstream m_file;
        int m_index{0};
        std::string m_defaultCh {" "};
        bool m_sizeChange {true};
        bool m_sorted {false};

        void m_getWinSize();
        void m_initScreen();
        std::string m_genrate(c_cell& ch);
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
