#include <string>
#include <iostream>
#include <vector>
#include <sys/ioctl.h>
#include "../tui.h"

using namespace std;

Surface::Surface(array<int, 2> size, string ch, int z, array<int, 2> offset):z(z), r_cords{offset[0], offset[1]}, size{size[0], size[1]}{
    Character chr;
    chr.set_ch(ch);
    chr.genrate();
    surface.assign(size[0]*size[1], chr);
}
Surface::Surface(){
}

void Surface::fill_bg(int r, int g, int b){
    for(int i = 0; i != surface.size(); i++){
        surface[i].set_bg(r, g, b);
        surface[i].genrate();
    }
}
void Surface::fill_fg(int r, int g, int b){
    for(int i = 0; i != surface.size(); i++){
        surface[i].set_fg(r, g, b);
        surface[i].genrate();
    }
}
void Surface::set_z(int zi){
    z = zi;
}
int Surface::get_z(){
    return z;
}

array<int, 2> Surface::ssize(){
    return size;
}
array<int, 2> Surface::offset(){
    return r_cords;
}
void Surface::set_offset(int x, int y){
    r_cords[0] = x;
    r_cords[1] = y;
}
void Surface::blit(Surface &surf){
    int x = 0;
    int y = 0;
    for (int i = 0; i != surface.size(); i++){
        bool def {true};
        int r_x = x - surf.offset()[0];
        int r_y = y - surf.offset()[1];
        if (r_x < surf.ssize()[0] && r_y < surf.ssize()[1]){
            if (r_x >= 0 && r_y >= 0){
                int index = (surf.ssize()[0] * r_y) + r_x;
                if (!surf[index].bg_def){
                    surface[i].set_bg(surf[index].bg[0], surf[index].bg[1], surf[index].bg[2]);
                }
                if (!surf[index].fg_def){
                    surface[i].set_fg(surf[index].fg[0], surf[index].fg[1], surf[index].fg[2]);
                }
                if (!surf[index].ch_def){
                    surface[i].set_ch(surf[index].ch);
                }
                surface[i].genrate();
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

Character& Surface::operator[](int x){
    return surface[x];
}

