#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <fstream>
#include "api.h"

using namespace std;


int main(){
    ofstream log("log");
    clog.rdbuf(log.rdbuf());
    Tui tui;
    array<int, 2> offset {20, 12};
    array<int, 2> offset2 {20, 22};

    array<int, 3> BLACK {0, 0, 0};
    array<int, 3> WHITE {255, 255, 255};
    array<int, 3> BLUE {0, 0, 255};

    auto &surf = tui.append({40, 10}, offset);
    auto &surf2 = tui.append({40, 10}, offset2);
    auto &debug = tui.append({tui.getScreenSize()[0], tui.getScreenSize()[1]}, {-tui.getScreenSize()[0], -tui.getScreenSize()[1]}, " ", 1000);

    surf.fill_bg(210, 210, 210);
    surf.fill_fg(0, 0, 0);
    surf2.fill_bg(0, 255, 0);
    surf.register_keys({"q", "h", "j", "k", "l", "Up", "b"});

    string str = "😁Hello world😁\n😁Hello world😁";
    Label lab{surf, str, {2, 2}};

    string debugStr = "";
    Label debugLabel{debug, debugStr, {1, 1}};

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
            int cpf = tui.charactersRendered() / count;
            ostringstream s;
            s << "fps = " << static_cast<int>(fps) << " Total frames = " << count << "\n" <<  "Charecters per frame = " << cpf << " Total Charecters = " << tui.charactersRendered() << "\n" << "run time = " << static_cast<float>(diff) / 1000 << " secounds\n";
            debugStr = s.str();
            debugLabel.updateSurface();
        }

        if (surf.get_event("q")){
            break;
        }
        if (surf.get_event("d")){
            if (!show_debug){
                debug.set_offset(2, 2);
                show_debug = true;
            }else {
                debug.set_offset(-tui.getScreenSize()[0], -tui.getScreenSize()[1]);
                show_debug = false;
            }
        }
        if (surf.get_event("k")){
            offset[1] -= 1;
        }
        if (surf.get_event("j")){
            offset[1] += 1;
        }
        if (surf.get_event("h")){
            offset[0] -= 1;
        }
        if (surf.get_event("l")){
            offset[0] += 1;
        }
        surf.set_offset(offset[0], offset[1]);
        
        tui.update();
        count ++;

        // system("sleep .05");
    }
    system("clear");
    cout << "the amount of frames render was " << count << endl;
    cout << "the amount of charecters renderd was " << tui.charactersRendered() << endl;
 
}
