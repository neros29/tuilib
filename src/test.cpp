#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "core/tui.h"
#include <chrono>

using namespace std;


int main(){
    ofstream file("log", ios::app);
    clog.rdbuf(file.rdbuf());
    clog << "[Program] Starting up" << endl;
    Screen screen;
    Input input;

    array<int, 2> offset {20, 12};
    array<int, 2> offset2 {20, 22};

    array<int, 3> BLACK {0, 0, 0};
    array<int, 3> WHITE {255, 255, 255};
    array<int, 3> BLUE {0, 0, 255};

    auto &surf = screen.append({40, 10}, offset);
    auto &surf2 = screen.append({40, 10}, offset2);
    auto &debug = screen.append({screen.getSize()[0], screen.getSize()[1]}, {-screen.getSize()[0], -screen.getSize()[1]}, " ", 1000);

    surf.fill_bg(210, 210, 210);
    surf.fill_fg(0, 0, 0);
    surf2.fill_bg(0, 255, 0);

    string str = "😁Hello world😁\n😁Hello world😁";
    Label lab(surf, str, {2, 2});

    string debugStr = "";
    Label debugLabel(debug, debugStr, {1, 1});

    int color = 1;
    system("clear");
    // cout << "\x1b[3J";
    int vec[2] {1, 1};

    unsigned long count = 0;
    auto start = chrono::system_clock::now();

    bool show_debug = false;
    while (true){
        int diff = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count();
        if (diff > 1){
            double fps = (static_cast<double>(count) / static_cast<double>(diff)) * 1000;
            int cpf = screen.amount / count;
            ostringstream s;
            s << "fps = " << static_cast<int>(fps) << " Total frames = " << count << "\n" <<  "Charecters per frame = " << cpf << " Total Charecters = " << screen.amount << "\n" << "run time = " << static_cast<float>(diff) / 1000 << " secounds\n";
            debugStr = s.str();
            debugLabel.updateSurface();
        }
        //
        char ch = input.get_char();
        if (ch == 'q' || ch == 3){
            break;
        }
        else if (ch == 'd'){
            if (show_debug){
                debug.set_offset(-screen.getSize()[0], -screen.getSize()[1]);
                show_debug = false;
            }
            else{
                debug.set_offset(2, 2);
                show_debug = true;
            }
        }

        else if (ch == 'r'){
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
        else if (ch == 'j'){
            offset[1] ++;
        }
        else if (ch == 'k'){
            offset[1] --;
        }
        else if (ch == 'h'){
            offset[0] --;
        }
        else if (ch == 'l'){
            offset[0] ++;
        }
        else if (ch == '2'){
            surf.set_z(2);
        }
        else if (ch == '0'){
            surf.set_z(0);
        }
        surf.set_offset(offset[0], offset[1]);
        // surf2.set_offset(offset[0], offset[1]- 10);
        screen.flip();
        count ++;

        // system("sleep .05");
    }
    system("clear");
    clog << "[Program] Shutting down" << endl;
    file.close();
    cout << "the amount of frames render was " << count << endl;
    cout << "the amount of charecters renderd was " << screen.amount << endl;
 
}
