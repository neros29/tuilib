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
void Surface::set_offset(int x, int y){
    r_cords[0] = x;
    r_cords[1] = y;
}
void Surface::add_string(string s, int col_start, int row, int fg[3], int bg[3]){
    int x = 0;
    int y = 0;
    InputString str(s);
    cout << str[0]<< endl;
    for (int i = 0; i != surface.size(); i++){
        int r_x = x - col_start;
        int r_y = y - row; 
        if (r_x < str.size() && r_y < 1){
            if (r_x >= 0 && r_y >= 0){
                int index = (str.size() * r_y) + r_x;
                if (fg != nullptr && fg[0] != -1){
                    surface[i].set_fg(fg[0], fg[1], fg[2]);
                }if (bg != nullptr && bg[0] != -1){
                    surface[i].set_bg(bg[0], bg[1], bg[2]);
                }
                surface[i].set_ch(str[index]);
                surface[i].genrate();
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

