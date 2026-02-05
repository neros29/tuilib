#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <fstream>
#include "api.h"

using namespace std;


int main(){
    ofstream log("log", ios::app);
    clog.rdbuf(log.rdbuf());
    clog << "\n=====================================================\n" << endl;;
    clog << "[Program] Starting" << endl;;

    Tui tui;

    array<int, 2> offset {20, 12};
    array<int, 2> offset2 {20, 22};

    auto &surf = tui.append({40, 10}, offset);
    surf.fill_fg(0xDE, 0xDC, 0xD7);
    surf.fill_bg(0x3,0x3, 0x5);
    surf.register_keys({"q", "h", "j", "k", "l", "d", "Up", "Down", "Left", "Right"});
    string str = "💀Hello world💀\n\n💀Hello world💀";
    Label lab{surf, str, {5, 2}};

    auto &surf2 = tui.append({40, 10}, offset2);
    surf2.fill_bg(0x57, 0xA4, 0x90);

    auto &debug = tui.append({tui.getScreenSize()[0], tui.getScreenSize()[1]}, {-tui.getScreenSize()[0], -tui.getScreenSize()[1]}, " ", 1000);
    debug.fill_fg(0xDE, 0xDC, 0xD7);
    string debugStr = "";
    Label debugLabel{debug, debugStr, {1, 1}};



    
    system("clear");

    unsigned long frameCount = 0;

    short color = 1;
    bool show_debug = false;
    unsigned long long avgFrameTime = 0;

    clog << "[Program] Entering main loop" << endl;;
    auto start = chrono::system_clock::now();
    while (true){
        int diff = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count();
        if ((diff*1000) % (60/20) == 0 && frameCount > 0){
            double fps = (static_cast<double>(frameCount - tui.skipedFrames) / static_cast<double>(diff)) * 1000;
            int cpf = tui.charactersRendered() / (frameCount - tui.skipedFrames);
            ostringstream s;
            s << "fps = " << static_cast<int>(fps) << " Total frames = " << frameCount << "\n" <<  "Charecters per frame = " << cpf << " Total Charecters = " << tui.charactersRendered() << "\n" << "run time = " << static_cast<float>(diff) / 1000 << " secounds\n";
            s << "avg Frame Time " << (static_cast<float>(avgFrameTime) / static_cast<float>(frameCount)) / 1000000.0f  << " ms\n";

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
        
        auto before = chrono::system_clock::now();
        tui.update();
        auto total = chrono::system_clock::now();
        avgFrameTime += chrono::duration_cast<chrono::nanoseconds>(total - before).count();

        frameCount ++;

        // system("sleep .05");
    }
    clog << "[Program] Ending" << endl;;
    system("clear");
    cout << "the amount of frames render was " << frameCount - tui.skipedFrames << endl;
    cout << "the avargae update time was " << (static_cast<float>(avgFrameTime) / static_cast<float>(frameCount)) / 1000000.0f   << endl;
    cout << "the amount of frames skipped was " << tui.skipedFrames << endl;
    cout << "the amount of charecters renderd was " << tui.charactersRendered() << endl;
 
}
