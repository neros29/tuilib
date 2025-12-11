#include <iostream>
#include <string>
#include "tui.h"
#include <time.h>

using namespace std;
int main(){
    Screen screen;
    Input input;
    int size[2] {40, 10};
    int offset[2] {1, 5};

    auto &surf = screen.append(size, " ", 0, offset);

    surf.fill_bg(255, 255, 255);
    surf.fill_fg(0, 0, 0);
    surf.add_string("Hello World", (size[0]/2) - 6 , 4);

    int color = 1;
    system("clear");
    cout << "\x1b[3J";
    int vec[2] {1, 1};
    unsigned long count = 0;
    time_t start = time(nullptr);
    time_t secounds = 10;


    while (true){
        // time_t diff = time(nullptr) - start;
        // if (diff > secounds){
        //     break;
        // }
        char ch = input.get_char();

        if (ch == 'q' || ch == 3){
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
        count ++;

        // system("sleep .05");
    }
    system("clear");
    cout << "the amount of frames render was " << count << endl;
    cout << "the amount of charecters renderd was " << screen.amount << endl;
 
}
