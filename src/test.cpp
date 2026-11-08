#include <array>
#include <fstream>
#include <string>
#include <term.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include "core/tui.h"
#include "core/utils/logging.h"
#include <chrono>
#include "api.h"
#include "widgets/Label.h"

using namespace std;
class Debug{
    Surface surf;
    string str;
    std::vector<pair<string, array<int, 2>>> vec;
public:

    Debug(Surface s, string t): surf(std::move(s)), str(t){
        surf.fill_bg(0, 0, 0, 100);
        surf.fill_fg(0xbf, 0xc1, 0xc8);
        init_text();
    }

    int render_text(int start, string text, int length = -1){
        if (length == -1){
            length = text.size();
        }
        for (int i = 0; i <= length; i ++){
            if (i < text.size()){
                surf[i + start].set_ch(string{text[i]});
            } else {
                surf[i + start].set_ch(" ");
            }
        }
        return start + length;
    }

    pair<string, int> parse_braks(string buff){
        string name {""};
        string length {""};
        bool in_name {true};
        bool in_len {false};
        for (char ch : buff){
            if (in_len){
                if ( ch == '}'){
                    break;
                }
                length += ch;
            }
            else if (in_name){
                if (ch == '{'){
                    continue;
                }
                if (ch == ':'){
                    in_len = true;
                    in_name = false;
                    continue;
                } 
                name += ch;
            }
        }
        int len = std::stoi(length);
        return {name, len};
    };

    void init_text(){
        int start {0};
        int l {0};
        bool in_brak = false;
        string buff;
        for (char ch : str){
            if (in_brak){
                if (ch == '}'){
                    auto b = parse_braks(buff); 
                    pair<string, array<int, 2>> p {b.first, {start, b.second}};
                    vec.push_back(p);
                    start += b.second;
                    buff.clear();
                    in_brak = false;
                }
                buff += ch;
                continue;
            }
            if (ch == '\n'){
                start = render_text(start, buff);
                l += 1;
                start = l * surf.get_size()[0];
                buff.clear();
                continue;
            }
            if (ch == '{'){
                in_brak = true;
                start = render_text(start, buff);
                buff.clear();
                continue;
            }
            buff += ch; 
        }
        if (!buff.empty()){
            render_text(start, buff);
        }
    };
    void render_valus(unordered_map<string, string>& values){
        for (pair<string, array<int, 2>>& p : vec){
            render_text(p.second[0], values[p.first], p.second[1]);
        }
    }
    bool ToggleShown(){
        if (surf.get_offset()[0] < 0){
            surf.set_offset(0, 0);
            return true;
        } else {
            surf.set_offset(-surf.get_size()[0], -surf.get_size()[1]);
            return false;
        }
    };
    void draw(unordered_map<string, string>& values){
        if (surf.get_offset()[0] < 0){
            return;
        } else {
            render_valus(values);
        }
    }
};


class Main{
    Tui tui;
    Surface event;
    bool running {true};
    double fps {144.0};
    
    string debug_format_str = 
R"(Debug Screen
FPS: {FPS:7}
Run Time: {RUN_TIME:7}s 
Total Frames Rendered: {FRAMES_RENDERED:5}
Characters drawn per frame: {CHARACTERS_DRAWN:5}
Frames Skipped: {FRAMES_SKIPPED:5}
)";
    Debug debug_surf;
    bool debug_is_shown {false};
    unordered_map<string, string> values;

    Surface surf1;
    string surf1_text = "Unicode test.\nそれん\nソレン\nSøren";
    Label lab1;

    array<int, 2> surf2_offset = {20, 12};
    Surface surf2;
    string surf2_text = "Terrminal User Interface\nbuilt by Soren Mile";
    Label lab2;

    bool events(Event e){
        unordered_map<std::string, array<int, 2>> text_dirs {
            {"k", {0, -1}},
            {"j",  {0, 1}},
            {"h", {-2, 0}},
            {"l",  {2, 0}},
        };
        unordered_map<Key, array<int, 2>> key_dirs {
            {Key::Up, {0, -1}},
            {Key::Down,  {0, 1}},
            {Key::Left, {-2, 0}},
            {Key::Right,  {2, 0}},
        };
        bool results = false;
        if (e.kind == Event::Kind::Text){
            if (e.text == "q"){
                running = false;
                results = true;
            }
            if (e.text == "d"){
                debug_is_shown = debug_surf.ToggleShown();
                results = true;
            }
            auto itr = text_dirs.find(e.text);
            if (itr != text_dirs.end()){
                surf2_offset[0] += text_dirs[e.text][0];
                surf2_offset[1] += text_dirs[e.text][1];
                results = true;
            }
        }
        if (e.kind == Event::Kind::Key){
            auto itr = key_dirs.find(e.key);
            if (itr != key_dirs.end()){
                surf2_offset[0] += key_dirs[e.key][0];
                surf2_offset[1] += key_dirs[e.key][1];
                results = true;
            }
        }
        surf2.set_offset(surf2_offset[0], surf2_offset[1]);
        return results;
    }
    void draw(){
        if (debug_is_shown){
            debug_surf.draw(values);
        }
        tui.update(16);
    }
public:
    Main(): 
        tui(),
        event(tui.addSurface({0, 0}, {0, 0}, " ", 1000)),
        debug_surf(tui.addSurface(tui.getScreenSize(), {-tui.getScreenSize()[0], -tui.getScreenSize()[1]}, " ", 100), debug_format_str),
        surf1(tui.addSurface({40, 10}, {20, 22}, " ", 70)),
        lab1(surf1, surf1_text, {5, 2}), 
        surf2(tui.addSurface({40, 10}, surf2_offset, " ", 80)),
        lab2(surf2, surf2_text, {5, 2})
    {
        cout << "\x1b[?1049h" << endl;
        event.register_func([this](Event e){
                return events(e);
                });
        surf1.fill_fg(0xbf, 0xc1, 0xc8, 200);
        surf1.fill_bg(0x57, 0xA4, 0x90, 200);

        surf2.fill_fg(0xbf, 0xc1, 0xc8);
        surf2.fill_bg(0x10,0x50, 0x10, 200);
        surf2.fill_style(styles::UNDERLINE | styles::BOLD);
        logger::info("[Main] starting up.");
    }
    ~Main(){
        cout << "\x1b[?1049l" << endl;
    }
    void run(){
        double alpha = 0.05;
        Tui::Clock clock(fps, alpha);
        auto start = chrono::high_resolution_clock::now(); 
        unsigned long frame_count {0};
        double cpf = 0.0;
        while (running){
            if (debug_is_shown){
                auto frame_start = chrono::high_resolution_clock::now(); 
                if (frame_count > 0){
                    double t = tui.charactersRendered() / static_cast<double>(frame_count);
                    cpf = (alpha * t) + ((1.0 - alpha) * cpf);
                    values["CHARACTERS_DRAWN"] = to_string(cpf);
                }
                values["FPS"] = to_string(clock.get_fps());
                values["RUN_TIME"] = to_string(chrono::duration_cast<chrono::milliseconds>(frame_start - start).count() / 1000.0);
                values["FRAMES_RENDERED"] = to_string(frame_count);
                values["FRAMES_RENDERED"] = to_string(frame_count);
                values["FRAMES_SKIPPED"] = to_string(tui.skipedFrames);
            }
            draw();
            frame_count += 1;
            clock.tick();
        }
    }
};
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
    log << "[" << level_str << "] " << "'" <<msg << "'" <<endl;
    log.flush();
    log.close();
}

static void clear(){
    char code[] {"\x1b[2J\x1b[H"};
    write(1, &code, sizeof(code));
}

int main(){
    Logger::set_callback(log);
    logger::info("================================= PROGRAM STARTING =================================");
    // clear();
    {
        Main main;
        main.run();
    }
    logger::info("Program Ending");
    // clear();
}
