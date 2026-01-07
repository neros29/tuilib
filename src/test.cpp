#include <iostream>
#include <string>
#include "tui.h"
#include <time.h>

using namespace std;
int main(){
    Screen screen;
    Input input;
    array<int, 2> offset {1, 12};
    array<int, 2> offset2 {1, 22};

    array<int, 3> BLACK {0, 0, 0};
    array<int, 3> WHITE {255, 255, 255};
    array<int, 3> BLUE {0, 0, 255};


    auto &surf = screen.append({40, 10}, offset);
    auto &surf2 = screen.append({40, 10}, offset2);

    // int of[2] {0, 0};
    // int sz[2] {5, 5};
    // for (int i = 0; i < 255; i ++){
    //     auto &s = screen.append(sz, of);
    //     s.fill_bg(i, i, i);
    //     of[0] = i;
    //
    // }
    //
    //
    surf.fill_bg(210, 210, 210);
    surf.fill_fg(0, 0, 0);
    surf2.fill_bg(0, 255, 0);


    Surface sub({10, 5}, " ", 0, {3, 3});
    sub.fill_bg(255, 255, 0);
    int color = 1;
    system("clear");
    cout << "\x1b[3J";
    int vec[2] {1, 1};
    unsigned long count = 0;
    time_t start = time(nullptr);
    time_t secounds = 10;


    while (true){
        time_t diff = time(nullptr) - start;
        if (diff > secounds){
            break;
        }
        char ch = input.get_char();

        surf.blit(sub);
        if (ch == 'q' || ch == 3){
            break;
        }
        if (ch == 'r'){
            if(color == 1){
                surf.fill_bg(0, 0, 0);
                surf.fill_fg(255, 255, 255);
                surf[0].set_bg(255, 0, 0);
                surf[0].genrate();
                color = 0;
            }
            else{
                surf.fill_bg(255, 255, 255);
                surf.fill_fg(0, 0, 0);
                color = 1;
            }
        }
        if (ch == 'j'){
            offset[1] ++;
        }
        if (ch == 'k'){
            offset[1] --;
        }
        if (ch == 'h'){
            offset[0] --;
        }
        if (ch == 'l'){
            offset[0] ++;
    
        }
        if (ch == '2'){
            surf.set_z(2);
        }
        if (ch == '0'){
            surf.set_z(0);
        }
        surf.set_offset(offset[0], offset[1]);
        // surf2.set_offset(offset[0], offset[1]- 10);
        screen.flip();
        count ++;

        // system("sleep .05");
    }
    system("clear");
    cout << "the amount of frames render was " << count << endl;
    cout << "the amount of charecters renderd was " << screen.amount << endl;
 
}
