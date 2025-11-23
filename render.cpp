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
    int id;
    Surface(int size[2], string ch, int z, int offsett[2]):z(z), r_cords{offsett[0], offsett[1]}, size{size[0], size[1]}{
        Character chr;
        chr.set_ch(ch);
        chr.genrate();
        surface.assign(size[0]*size[1], chr);
        cout << size[0] * size[1] << endl;
        cout << surface.size() << endl;
    }
    Character operator[](int x){
        return surface[x];
    }
    void fill(int r, int g, int b){
        cout << surface.size() << endl;
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
    vector <int> sort_idx;
    int idx {0};
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
    
    void sort_surfaces(){
        sort(sort_idx.begin(), sort_idx.end(), 
            [&](const int& a, const int& b){
                return surfaces[a].z > surfaces[b].z;
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
            sort_surfaces();
        }
        if (!size_ch){
            init_screen();
        }
        int x = 0;
        int y = 0;
        for (int i = 0; i != screen.size(); i++){
            for (int j: sort_idx){
                Surface &surf = surfaces[j];
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

    Surface &append(int size[2], string ch, int z, int offset[2]){
        Surface surf(size, ch, z, offset);
        surfaces.push_back(surf);
        sort_idx.push_back(idx);
        idx++;
        soreted = false;
        return surfaces[idx-1];
    }

    void flip(){
        ofstream file("/dev/stdout");
        render();
        file << "\x1b[?25l";
        for (int i = 0; i != screen.size(); i++){
            int y = i / size[0];
            int x = i % size[0];
            // // if (screen[i] != def_chr){
            // //     file <<"\x1b[" << y << ";"<< x << "H" << screen[i];
            // // } 
            // // file <<"\x1b[" << y << ";"<< x << "H" << screen[i];
        }
        file <<"\x1b[?25h";
        file.flush();
    }
};

int main(){
    Screen screen;
    int size[2] = {10, 10};
    int offset[2] = {0, 0};
    int offset2[3] = {8, 0};
    int offset3[3] = {0, 8};
    auto &surf = screen.append(size, "&", 1, offset);
    auto &surf2 = screen.append(size, "#", 0, offset2);
    auto &surf3 = screen.append(size, " ", 1, offset3);
    surf3.fill(255, 255, 255);
    // // cout << "\x1b[3J";
    while(true){
        // auto start = chrono::high_resolution_clock::now();
        screen.flip();
        // auto end = chrono::high_resolution_clock::now();
        // auto taken = chrono::duration_cast<chrono::milliseconds>(end - start);
        // cout << "\n" << taken.count() << "\r";
    }
}
