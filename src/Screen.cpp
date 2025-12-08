#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <deque>

#include "tui.h"




void Screen::get_win_size(){
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
    
void Screen::sort_surfaces(){
    sort(sort_idx.begin(), sort_idx.end(), 
        [&](const int& a, const int& b){
            return surfaces[a].z > surfaces[b].z;
        }
    );
    soreted = true;
};
void Screen::init_screen(){
    screen.assign(size[0]*size[1], def_chr);
}
void Screen::render(){
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

int size[2];

Surface &Screen::append(int size[2], string ch, int z, int offset[2]){
    Surface surf(size, ch, z, offset);
    surfaces.push_back(surf);
    sort_idx.push_back(idx);
    idx++;
    soreted = false;
    return surfaces[idx-1];
}

void Screen::flip(){
    ofstream file("/dev/stdout");
    render();
    file << "\x1b[?25l";
    for (int i = 0; i != screen.size(); i++){
        int y = i / size[0];
        int x = i % size[0];
        if (screen[i] != def_chr){
            file <<"\x1b[" << y << ";"<< x << "H" << screen[i];
        } 
        file <<"\x1b[" << y << ";"<< x << "H" << screen[i];
    }
    file <<"\x1b[?25h";
    file.flush();
}
