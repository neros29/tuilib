#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <algorithm>
#include <fstream>
#include <chrono>

using namespace std;



class Character{
private:
    bool bg_def = true;
    bool fg_def = true;
    int fg[3];
    int bg[3];

public:
    string ch = "";
    string ansii {""};
    void set_fg(int r, int g, int b){
        fg[0] = r;
        fg[1] = g;
        fg[2] = b;
        fg_def = false;
    }
    void set_bg(int r, int g, int b){
        bg[0] = r;
        bg[1] = g;
        bg[2] = b;
        bg_def = false;
    }
    void set_ch(string chi){
        ch = chi;
    }
    void genrate(){
        string ESC {"\x1B"};
        ostringstream oss;
        ostringstream _fg;
        ostringstream _bg;
        if (!fg_def){
            _fg << ESC << "[38;2;"<< fg[0] << ";" << fg[1] << ";"<< fg[2] <<"m";
        }
        if (!bg_def){
            _bg << ESC << "[48;2;"<< bg[0] << ";" << bg[1] << ";" << bg[2] << "m";
        }
        oss << _bg.str() << _fg.str() << ch << ESC << "[0m";
        ansii = oss.str();
    }
};

class Surface{
public:
    vector <Character> surface;
    int r_cords[2];
    int size[2];
    int z {0};

    Surface(int size[2], string ch, int z, int x, int y):z(z), r_cords{x, y}, size{size[0], size[1]}{
        Character chr;
        chr.set_ch(ch);
        chr.genrate();
        surface.assign(size[0]*size[1], chr);

    }
    Character operator[](int x){
        return surface[x];
    }
    void fill(int r, int g, int b){
        for(int i = 0; i != surface.size(); i++){
            surface[i].set_bg(r, g, b);
            surface[i].genrate();
        }
    }
};

class Screen{
private:
    vector <Surface> surfaces;
    vector <string> screen;
    string def_chr = " ";
    bool soreted {false};
    bool size_ch {true};
    
    void get_win_size(){
        struct winsize ws = {(struct winsize){0,0,0,0}};
        ioctl(0, TIOCGWINSZ, &ws);
        if (size[0] != ws.ws_col){
            size[0] = ws.ws_col;
            size_ch = true;
        }
        if (size[1] != ws.ws_row){
            size[1] = ws.ws_row;
            size_ch = true;
        }
        size_ch = false;
    }
    void sort_surfaces(vector <Surface> surf){
        sort(surf.begin(), surf.end(), 
            [](const Surface& a, const Surface& b){
            return a.z < b.z;
            }
        );
        soreted = true;
    };
    void init_screen(){
        screen.assign(size[0]*size[1], def_chr);
    }
    void render(){
        get_win_size();
        if (!soreted){
            sort_surfaces(surfaces);
        }
        if (!size_ch){
            init_screen();
        }

        int x = 0;
        int y = 0;
        for (int i = 0; i != screen.size(); i++){
            for (Surface &surf: surfaces){
                int r_x = x - surf.r_cords[0];
                int r_y = y - surf.r_cords[1];

                if (r_x < surf.size[0] && r_y < surf.size[1]){
                    if (r_x >= 0 && r_y >= 0){
                        int index = (surf.size[0] * r_y) + r_x;
                        screen[i] = surf[index].ansii;
                        break;
                      
                    
                    }
                    
                }
                
            }
            if (x == size[0]-1){
                x = 0;
                y ++;
            }
            else{
                x++;
            }

        }        
    }

public:
    int size[2];
    void append(Surface &surf){
        surfaces.push_back(surf);
        soreted = false;
    }
    
    void flip(){
        ofstream file("/dev/stdout");
        
        render();
        file << "\x1b[?25l";
        for (int i = 0; i != screen.size(); i++){
            int y = i / size[0];
            int x = i % size[0];
            // if (screen[i] != def_chr){
            //     file <<"\x1b[" << y << ";"<< x << "H" << screen[i];
            //} 
            file <<"\x1b[" << y << ";"<< x << "H" << screen[i];
       
        }
        file <<"\x1b[?25h";
        file.flush();
        
    }

};

int main(){
    Screen screen;
    int size[2] = {10, 10};
    int x = 0;
    int y = 0;
    Surface surf(size, " ", 1, x, y);
    screen.append(surf);
    surf.fill(255, 0, 0);
    cout << "\x1b[3J";
   while(true){
       screen.flip();
        x++;
        y++;
    }
}
