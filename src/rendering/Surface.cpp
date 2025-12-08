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

void Surface::fill(int r, int g, int b){
    for(int i = 0; i != surface.size(); i++){
        surface[i].set_bg(r, g, b);
        surface[i].genrate();
    }
}
Character Surface::operator[](int x){
    return surface[x];
}

