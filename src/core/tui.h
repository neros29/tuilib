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

struct Character {
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
    bool operator == (const Character ch) const;
};

class InternalSurface {
public:
    vector<Character> surface;
    vector<Character> m_old;

    int z{0};
    bool m_dirty = true;
    array<int, 2> size;
    array<int, 2> r_cords;

    const vector<string>* m_validKeys;
    unordered_map<string, bool> events;
    vector<string> keys;

    InternalSurface(array<int, 2> size, string ch, int z, array<int, 2> offset, const vector<string>& keys);
    InternalSurface(array<int, 2> size, string ch, int z, array<int, 2> offset);
    Character& operator[](int x);
    Character operator[](int x) const;
    bool cheakDirty();
};



class Screen {
private:
    const deque<InternalSurface>& m_surfaces;
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
    int charactersRenderd;
    Screen(const deque<InternalSurface>& surfaces, const vector<int>& sortIndex);
    ~Screen();
    array<int, 2> getSize();
    void flip();
};

// Input
class Input{
private:
    // Surface data passed from api 
    deque<InternalSurface>& m_surfaces; 
    const vector<int>& m_sortIndex;

    // input stuff
    struct termios m_old;
    vector<array<string, 2>> m_namePair;
    unordered_map<string, string> m_termDb;

    // input buffer
    string m_buffer;

    // member private functions
    inline void m_initNamePair();
    void m_setRawMode();
    void m_restoreMode();
    void m_initDb();
    void m_add(string capname, string human);
    string m_getCap(const char* name);
public:
    vector<string> keys;
    // Constructers
    Input(deque<InternalSurface>& surfaces, const vector<int>& sortIndex);
    ~Input();

    // member public functions
    bool update();
};

