#include <iostream>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <algorithm>
#include <fstream>
#include <deque>

#include "../tui.h"

void Screen::get_win_size(){
    struct winsize ws = {(struct winsize){0,0,0,0}};
    ioctl(0, TIOCGWINSZ, &ws);
    if (size[0] != ws.ws_col + 1 || size[1] != ws.ws_row + 1){
        size[0] = ws.ws_col + 1;
        size[1] = ws.ws_row + 1;
        size_ch = true;
    }
    else {
        size_ch = false;
    }
}

void Screen::sort_surfaces(){
    sort(sort_idx.begin(), sort_idx.end(), 
        [&](const int& a, const int& b){
            return surfaces[a].get_z() > surfaces[b].get_z();
        }
    );
    sorted = true;
}
vector<int> Screen::get_size(){
    vector<int> size_v {size[0], size[1]};
    return size_v;
}

void Screen::init_screen(){
    screen.assign(size[0] * size[1], def_chr);
    last_screen.assign(size[0] * size[1], def_chr);
}

void Screen::render(){
    get_win_size();
    sort_surfaces();
    // if (!sorted){
    //     sort_surfaces();
    // }
    if (size_ch){
        init_screen();
    }
    int x = 0;
    int y = 0;
    for (int i = 0; i != screen.size(); i++){
        bool def {true};
        for (int j: sort_idx){
            Surface &surf = surfaces[j];
            int r_x = x - surf.offset()[0];
            int r_y = y - surf.offset()[1];
            if (r_x < surf.ssize()[0] && r_y < surf.ssize()[1]){
                if (r_x >= 0 && r_y >= 0){
                    int index = (surf.ssize()[0] * r_y) + r_x;
                    screen[i] = surf[index].ansii;
                    def = false;
                    break;
                }
            }
        }
        if (def){
            screen[i] = def_chr;
        }
        if (x == size[0] - 1){
            x = 0;
            y ++;
        }
        else{
            x++;
        }
    }
}

Surface &Screen::append(array<int, 2> size,  array<int, 2> offset, string ch, int z){
    if (z == -1){
        z = idx;
    }
    Surface surf(size, ch, z, offset);
    surfaces.push_back(surf);
    sort_idx.push_back(idx);
    idx++;
    sorted = false;
    return surfaces[idx-1];
}

void Screen::flip(){
    render();
    file << "\x1b[?25l";
    for (int i = 0; i != screen.size(); i++){
        int y = i / size[0];
        int x = i % size[0];
        if (screen[i] != last_screen[i]){
            file <<"\x1b[" << y << ";"<< x << "H" << screen[i];
            amount++;
        }
    }
    last_screen = screen;
    file <<"\x1b[?25h";
    file.flush();
}
Screen::Screen(){
    get_win_size();
    init_screen();
    file.open("/dev/stdout");
}
Screen::~Screen(){
    file.close();
}

