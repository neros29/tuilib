#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <fstream>
#include <unistd.h>
#include "api.h"
#include "core/tui.h"
#include "widgets/Label.h"
#include "core/utils/logging.h"


using namespace std;

static void clear(){
    char code[] {"\x1b[2J\x1b[H"};
    write(1, &code, sizeof(code));
}

static void too_many_surfaces(Tui& tui){
    for (int y = 0; y <= tui.getScreenSize()[1] ; y++){
        for (int x = 0; x <= tui.getScreenSize()[0]; x++){
            auto tmp = tui.addSurface({1, 1}, {x, y}, " ", x * y);
            int blue = x % 255;
            int red = y % 255;
            tmp.fill_bg(red, 0, blue);
        }
    }
}
static void one_big_surface(Tui& tui){
    auto surf = tui.addSurface({tui.getScreenSize()[0], tui.getScreenSize()[1]}, {0, 0}, " ");
    for (int y = 0; y < tui.getScreenSize()[1] ; y++){
        for (int x = 0; x < tui.getScreenSize()[0]; x++){
            int blue = x % 255;
            int red = y % 255;
            int index = y * surf.get_size()[0] + x;
            surf[index].set_bg(red, 0, blue);
        }
    }
}
static void good_amont_surfaces(Tui& tui){
    for (int x = 0; x <= tui.getScreenSize()[0]; x++){
        auto tmp = tui.addSurface({1, tui.getScreenSize()[1]}, {x, 1}, " ", x);
        int blue = x % 255;
        tmp.fill_bg(0, 0, blue);
    }

}
static void log(int level, const std::string& msg) {
    std::string level_str;
    ofstream log("logs/debug.log", ios::app);
    switch (level) {
        case LogLevel::INFO:     level_str = "INFO";     break;
        case LogLevel::WARNING:  level_str = "WARNING";  break;
        case LogLevel::ERROR:    level_str = "ERROR";    break;
        case LogLevel::CRITICAL: level_str = "CRITICAL"; break;
        case LogLevel::DEBUG:    level_str = "DEBUG";    break;
        default:                 level_str = "UNKNOWN";  break;
    }
    log << "[" << level_str << "] " << msg << endl;
}
void debug(){
    Logger::set_callback(log);
    logger::debug("\n=====================================================\n");
    logger::debug("[Program] Starting");

    Tui tui; // initialize tui
    logger::debug("[Program] Size of screen is " + to_string(tui.getScreenSize()[0]) + ", " + to_string(tui.getScreenSize()[1]));

    // defined offsets
    array<int, 2> offset {20, 12};
    array<int, 2> offset2 {20, 22};

    // surfaces stress test
    // too_many_surfaces(tui);
    // one_big_surface(tui);
    // good_amont_surfaces(tui);

    auto surf = tui.addSurface({40, 10}, offset);
    string str = "Terrminal User Interface\nbuilt by Soren Mile";
    surf.extend_size("Down", 5);
    surf.extend_size("Down", -5);
    surf.extend_size("Right", 10);
    surf.extend_size("Right", -10);
    surf.extend_size("Up", 10);
    surf.extend_size("Up", -10);
    surf.extend_size("Left", 10);
    surf.extend_size("Left", -10);
    surf.fill_style(styles::BOLD | styles::UNDERLINE);

    Label lab{surf, str, {5, 2}};
    surf.fill_fg(0xbf, 0xc1, 0xc8);
    surf.fill_bg(0x10,0x50, 0x10);
    surf.register_keys({"q", "h", "j", "k", "l", "d", "Up", "Down", "Left", "Right"});

    auto surf2 = tui.addSurface({40, 10}, offset2);
    surf2.fill_fg(0xbf, 0xc1, 0xc8, 200);
    surf2.fill_bg(0x57, 0xA4, 0x90, 200);
    string test = "Unicode test.\nそれん\nソレン";
    Label lab2{surf2, test, {5, 2}};

    auto debug = tui.addSurface({tui.getScreenSize()[0], tui.getScreenSize()[1]}, {-tui.getScreenSize()[0], -tui.getScreenSize()[1]}, " ");
    debug.fill_fg(0xbf, 0xc1, 0xc8);
    debug.fill_bg(0, 0, 0, 100);
    string debugStr = "";
    Label debugLabel{debug, debugStr, {0, 0}};

    
    clear();
    unsigned long frameCount = 0;

    short color = 1;
    bool show_debug = false;
    unsigned long long avgFrameTime = 0;
    logger::debug("[Program] Entering main loop");
    auto start = chrono::system_clock::now();
    while (true){
        int diff = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - start).count();
        if ((diff*1000) % (60/20) == 0 && frameCount > 0){
            double fps = (static_cast<double>(frameCount) / static_cast<double>(diff)) * 1000;
            int cpf = tui.charactersRendered() / (frameCount - tui.skipedFrames);
            ostringstream s;
            s << "Debug Screen\n";
            s << "fps = " << static_cast<int>(fps) << " Total frames = " << frameCount << "\n" <<  "Charecters per frame = " << cpf << " Total Charecters = " << tui.charactersRendered() << "\n" << "run time = " << static_cast<float>(diff) / 1000 << " secounds\n";
            s << "avg Frame Time " << (static_cast<float>(avgFrameTime) / static_cast<float>(frameCount)) / 1000000.0f  << " ms\n";
            s << "Total surfaces rendered " << tui.amontOfSurfaces() << endl;
            s << "Total characters in terminal " << tui.amontOfCharacters() << endl;
            debugStr = s.str();
            debugLabel.updateSurface();
        }
        if (surf.get_event("q")){
            break;
        }
        if (surf.get_event("d")){
            if (!show_debug){
                debug.set_offset(0, 0);
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
        // this_thread::sleep_for(chrono::milliseconds(3));
    }
    logger::debug("[Program] Ending");
    clear();
    cout << "the amount of frames render was " << frameCount - tui.skipedFrames << endl;
    cout << "the avargae update time was " << (static_cast<float>(avgFrameTime) / static_cast<float>(frameCount)) / 1000000.0f   <<  " ms"<< endl;
    cout << "the amount of frames skipped was " << tui.skipedFrames << endl;
    cout << "the amount of charecters renderd was " << tui.charactersRendered() << endl;
}

int main(int argc, char* argv[]){
    debug();
}
