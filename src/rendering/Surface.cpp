#include <string>
#include <vector>
#include <sys/ioctl.h>
#include "../tui.h"

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

void Surface::add_string(string str, int col_start, int row){
    int x = 0;
    int y = 0;
    for (int i = 0; i != surface.size(); i++){
        int r_x = x - col_start;
        int r_y = y - row; 
        if (r_x < str.size() && r_y < 1){
            if (r_x >= 0 && r_y >= 0){
                int index = (str.size() * r_y) + r_x;
                string ch = "";
                ch += str[index];
                surface[i].set_ch(ch);
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

