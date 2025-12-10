#include <iostream>
#include <string>
#include "tui.h"

using namespace std;
int main(){
    Screen screen;

    int size[2] {40, 10};
    int offset[2] {1, 5};

    auto &surf = screen.append(size, " ", 0, offset);

    surf.fill_bg(255, 255, 255);
    surf.fill_fg(0, 0, 0);
    surf.add_string("hello world", (size[0]/2) - 6 , 4);

    int color = 1;
    cout << "\x1b[3J";
    int vec[2] {1, 1};

    while (true){
        char ch = get_char();

        if (ch == 'q'){
            return 0;
        }
        if (ch == 'r'){
            if(color == 1){
                surf.fill_bg(0, 0, 0);
                surf.fill_fg(255, 255, 255);
                color = 0;
            }
            else{
                surf.fill_bg(255, 255, 255);
                surf.fill_fg(0, 0, 0);
                color = 1;
            }
        }
        if (ch == 'j'){
            surf.r_cords[1] += 1;
        }
        if (ch == 'k'){
            surf.r_cords[1] -= 1;
        }
        if (ch == 'h'){
            surf.r_cords[0] -= 1;
        }
        if (ch == 'l'){
            surf.r_cords[0] += 1;
        }
        screen.flip();
    }
}
