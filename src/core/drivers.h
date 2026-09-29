#pragma once
#include "tui.h"
#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <string>

namespace drivers {

    class d_screen{
        public:
        virtual ~d_screen(){};
        virtual void d_draw(int x, int y, core::c_cell cell) = 0;
        virtual void d_startFrame() = 0;
        virtual void d_endFrame() = 0;
        virtual std::array<int, 2> d_getWinSize() = 0;
    };
    class d_test_screen : public drivers::d_screen{
        void d_draw(int x, int y, core::c_cell cell) override {
            std::cout << "drawing: " << x << ", " << y << std::endl;
        };
        void d_startFrame() override {
            std::cout << "start frame." << std::endl;
        }
        void d_endFrame() override {
            std::cout << "end frame." << std::endl;
        }
        std::array<int, 2> d_getWinSize() override {
            return {5, 5};
        }
    };

    class d_defualt_screen : public drivers::d_screen{
        std::ofstream m_file;
        std::array<int, 2> m_size;
        std::string  m_genrate(core::c_cell& ch){
            static std::string ESC {"\x1B"};
            static std::string colors[256];
            if (colors[1].empty()){
                for (int i = 0; i <= 255; i++){
                    colors[i] = std::to_string(i) + ";";
                }
            }
            std::string ansii;
            ansii = "\x1b[0m\x1b[";

            std::array<u_int8_t, 4> fg = ch.get_fg();
            if (!ch.is_def("fg")){
                ansii += "38;2;";
				ansii += colors[fg[0]];
				ansii += colors[fg[1]];
				ansii += colors[fg[2]];
            }
            std::array<u_int8_t, 4> bg = ch.get_bg();
            if (bg[3] > 0 && !ch.is_def("bg")){
                ansii += "48;2;";
				ansii += colors[bg[0]];
				ansii += colors[bg[1]];
				ansii += colors[bg[2]];
            }
            styles style = static_cast<styles>(ch.get_style());
            if (style != styles::None){
				if (style & styles::BOLD)          ansii += "1;";
				if (style & styles::DIM)           ansii += "2;";
				if (style & styles::ITALIC)        ansii += "3;";
				if (style & styles::UNDERLINE)     ansii += "4;";
				if (style & styles::BLINKING)      ansii += "5;";
				if (style & styles::INVERSE)       ansii += "7;";
				if (style & styles::HIDDEN)        ansii += "8;";
				if (style & styles::STRIKETHROUGH) ansii += "9;";
            }
            ansii += "m";
            if (!ch.is_def("ch") && fg[3] > 0){
                ansii += ch.get_ch();
            } else{
                ansii += " ";
            }
            return ansii;
        }
    public:
        d_defualt_screen(){
            m_file.open("/dev/stdout");
        };
        ~d_defualt_screen() override {
            m_file.close();
        };
        void d_draw(int x, int y, core::c_cell cell) override {
            m_file <<"\x1b[" << y << ";"<< x  << "H" << m_genrate(cell);
        };
        void d_startFrame() override {
            m_file << "\x1b[?25l";
        }
        void d_endFrame() override {
            // m_file << "\x1b[?25l";
            m_file << "\x1b[0m";
            m_file <<"\x1b[?25h";
            m_file << "\x1b[" << m_size[1] + 1 << ";" << m_size[0] + 1 << "H";
            m_file.flush();
        }
        std::array<int, 2> d_getWinSize() override {
            struct winsize ws = {(struct winsize){0,0,0,0}};
            ioctl(0, TIOCGWINSZ, &ws);
            if (m_size[0] != ws.ws_col + 1 || m_size[1] != ws.ws_row + 1){
                m_size[0] = ws.ws_col + 1;
                m_size[1] = ws.ws_row + 1;
                return m_size;
            }
            else {
                return m_size;
            }
        }
    };
}
