#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <array>
#include <termios.h>
#include <term.h>

using namespace std;

class InputString{
private:
    int get_byte_len(unsigned char firstbyte);
    vector<string> str;
public:
    size_t size();
    int getWidth(string ch); 
    InputString(string s);
    string operator[](int i);
};

class Character {
public:
    bool bg_def {true};
    bool fg_def {true};
    bool ch_def {true};

    array<int, 3> fg;
    array<int, 3> bg;
    string ch = "";
    string ansii{""};

    void set_fg(int r, int g, int b);
    void set_bg(int r, int g, int b);
    void set_ch(string chi);
    void genrate();
};

class Surface {
private:
    array<int, 2> size;
    vector<Character> surface;
    array<int, 2> r_cords;
    int z{0};
public:
    unordered_map<string, bool> events;
    vector<string> keys;
    Surface(array<int, 2> size, string ch, int z, array<int, 2> offset);
    Surface();
    Character& operator[](int x);
    Character operator[](int x) const;
    bool get_event(string event);
    void register_keys(vector<string> keys);
    void fill_bg(int r, int g, int b);
    void fill_fg(int r, int g, int b);

    void set_z(int z);
    void set_offset(int x, int y);
    void blit(Surface& surf);

    int get_z() const;
    array<int, 2> get_offset() const;
    array<int, 2> get_size() const; 
};



class Screen {
private:
    const deque<Surface>& m_surfaces;
    vector<string> m_screen;
    vector<string> m_lastScreen;
    const vector<int>& m_sortIndex;
    array<int, 2> m_size;
    ofstream m_file;
    int m_index{0};
    string m_defaultCharacter {" "};
    bool m_sizeChange {true};
    bool m_sorted {false};

    void m_getWinSize();
    void m_initScreen();
    void m_render();

public:
    int amount; // amount of charecters renderd for debing
    Screen(const deque<Surface>& surfaces, const vector<int>& sortIndex);
    ~Screen();
    array<int, 2> getSize();
    void flip();
};

// Input
class Input{
private:
    struct termios g_old;
    vector<string> m_events;
    string m_buffer;
    unordered_map<string, string> m_termDb;
    deque<Surface>& m_surfaces; 
    const vector<int>& m_sortIndex;
    void set_raw_mode();
    void restore_mode();
    int getDataSize();
    void getData();
    void parseBuffer();
    void populateDb();
    string cap(const char* name); 
public:
    Input(deque<Surface>& surfaces, const vector<int>& sortIndex);
    ~Input();
    void update();
};

class Label{
private:
    Surface& root;
    Surface surf;
    const string& str;
    string oldStr;
    vector<string> text;
    array<int, 2> size;
    array<int, 2> offset;
    void m_getSize();
    void m_parseString(const string& i_str);
public:
    Label(Surface& i_root, const string& i_text, array<int, 2> i_offset);
    void updateSurface();
};
//
// class InputBox{
// private:
//     Surface& root;
//     Surface surf;
//     string& str;
//     array<int, 3> offset;
//
// public:
//     InputBox(Surface& i_root, string& i_str, array<int, 3> i_offset);
//     void updateSurface();
// };
