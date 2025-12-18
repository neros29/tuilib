#include <string>
#include <iostream>
#include <vector>
#include <sys/ioctl.h>
#include "../tui.h"

using namespace std;

Surface::Surface(int size[2], string ch, int z, int offsett[2]):z(z), r_cords{offsett[0], offsett[1]}, size{size[0], size[1]}{
    Character chr;
    chr.set_ch(ch);
    chr.genrate();
    surface.assign(size[0]*size[1], chr);
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

int* Surface::ssize(){
    return size;
}
int* Surface::offset(){
    return r_cords;
}
void Surface::set_offset(int x, int y){
    r_cords[0] = x;
    r_cords[1] = y;
}
void Surface::blit(Surface &surf, int row, int col){
    int x = 0;
    int y = 0;
    for (int i = 0; i != surface.size(); i++){
        bool def {true};
        int r_x = x - surf.r_cords[0];
        int r_y = y - surf.r_cords[1];
        if (r_x < surf.size[0] && r_y < surf.size[1]){
            if (r_x >= 0 && r_y >= 0){
                int index = (surf.size[0] * r_y) + r_x;
                surface[i] = surf[index];
            }
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

Character& Surface::operator[](int x){
    return surface[x];
}

