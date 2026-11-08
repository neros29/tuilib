#pragma once
#include "tui.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <string>
#include "utils/logging.h"
#include <unordered_map>
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
        // virtual int d_getCharacterWidth(uint32_t ch) = 0;
    };
    class d_default_input: public drivers::d_input{
    private:
        struct termios m_old;
        std::string m_buff;
        std::unordered_map<std::string, Key> m_keydb;
        std::unordered_map<char, char> m_asciiCtrldb;
        void m_addCtrl(char k, char v);
        void m_fillCtrldb();
        void m_fillDb();
        void m_add(std::string key, Key value);
        void m_setRawMode();
        void m_restoreMode();
    public:
        ~d_default_input() override;
        d_default_input();
        std::vector<Event> d_getEvents(int timeout_ms) override;
        // int d_getCharacterWidth(uint32_t ch) override;
    };
    class d_test_screen : public drivers::d_screen{
        void d_draw(int x, int y, const core::c_cell& cell) override {
            std::cout << "drawing: " << x << ", " << y << std::endl;
        };
        void d_startFrame() override {
            std::cout << "start frame." << std::endl;
        }
        void d_endFrame() override {
            std::cout << "end frame." << std::endl;
        }
        std::array<size_t, 2> d_getWinSize() override {
            return {5, 5};
        }
    };
    class d_defualt_screen : public drivers::d_screen{
        std::ofstream m_file;
        std::array<size_t, 2> m_size;
        core::c_cell m_last_cell;
        std::string m_buffer;

        static bool cell_style_equal(const core::c_cell& cell1, const core::c_cell& cell2){
            if (cell1.get_bg() != cell2.get_bg()){
                return false;
            }
            if (cell1.get_fg() != cell2.get_fg()){
                return false;
            }
            if (cell1.get_style() != cell2.get_style()){
                return false;
            }
            return true;
        }

        void  m_genrate(const core::c_cell& ch){
            static std::string colors[256];
            if (colors[1].empty()){
                for (int i = 0; i <= 255; i++){
                    colors[i] = std::to_string(i) + ";";
                }
            }
            m_buffer.append("\x1b[0m\x1b[");

            std::array<u_int8_t, 4> fg = ch.get_fg();
            if (!ch.is_def("fg")){
                m_buffer.append("38;2;");
				m_buffer.append(colors[fg[0]]);
				m_buffer.append(colors[fg[1]]);
				m_buffer.append(colors[fg[2]]);
            }
            std::array<u_int8_t, 4> bg = ch.get_bg();
            if (bg[3] > 0 && !ch.is_def("bg")){
                m_buffer.append("48;2;");
				m_buffer.append(colors[bg[0]]);
				m_buffer.append(colors[bg[1]]);
                m_buffer.append(colors[bg[2]]);
            }
            uint16_t style = ch.get_style();
            if (style != styles::NONE){
				if (style & styles::BOLD)          m_buffer.append("1;");
				if (style & styles::DIM)           m_buffer.append("2;");
				if (style & styles::ITALIC)        m_buffer.append("3;");
				if (style & styles::UNDERLINE)     m_buffer.append("4;");
				if (style & styles::BLINKING)      m_buffer.append("5;");
				if (style & styles::INVERSE)       m_buffer.append("7;");
				if (style & styles::HIDDEN)        m_buffer.append("8;");
				if (style & styles::STRIKETHROUGH) m_buffer.append("9;");
            }
            m_buffer.append("m");
            if (!ch.is_def("ch") && fg[3] > 0){
                m_buffer.append(ch.get_ch());
            } else{
                m_buffer += " ";
            }
        }
    public:
        d_defualt_screen(){
            m_file.open("/dev/stdout");
        };
        ~d_defualt_screen() override {
            m_file.close();
        };
        void d_draw(int x, int y, const core::c_cell& cell) override {
            if (cell.get_flags() & core::flags::PLACEHOLDER){
                return;
            }
            m_buffer.clear();
            if (cell_style_equal(cell, m_last_cell)){
                m_file <<"\x1b[" << y << ";"<< x  << "H" << cell.get_ch();
            } else{
                m_genrate(cell);
                m_file <<"\x1b[" << y << ";"<< x  << "H" << m_buffer;
                m_last_cell = cell;
            }
        };
        void d_startFrame() override {
            m_file << "\x1b[?25l";
            m_last_cell = core::c_cell();
        }
        void d_endFrame() override {
            m_file << "\x1b[0m";
            m_file <<"\x1b[?25h";
            m_file << "\x1b[" << m_size[1] + 1 << ";" << m_size[0] + 1 << "H";
            m_file.flush();
        }
        std::array<size_t, 2> d_getWinSize() override {
            struct winsize ws = {(struct winsize){0,0,0,0}};
            ioctl(0, TIOCGWINSZ, &ws);
            if (m_size[0] != ws.ws_col || m_size[1] != ws.ws_row){
                m_size[0] = ws.ws_col;
                m_size[1] = ws.ws_row;
                return m_size;
            }
            else {
                return m_size;
            }
        }
    };
}
