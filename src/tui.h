#pragma once
#include <deque>
#include <string>
#include <vector>
#include <fstream>
#include <array>
using namespace std;

//
//
// class InputString{
// private:
//     int get_byte_len(unsigned char firstbyte);
//     vector<string> str;
// public:
//     auto begin();
//     auto end();
//     size_t size();
//     InputString(string s);
//     string operator[](int i);
// };
//
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
    Character& operator[](int x);
    Surface(array<int, 2> size, string ch, int z, array<int, 2> offset);
    Surface();
    void fill_bg(int r, int g, int b);
    void fill_fg(int r, int g, int b);
    void set_z(int z);
    void set_offset(int x, int y);
    void blit(Surface& surf);

    int get_z();
    array<int, 2> offset();
    array<int, 2> ssize(); 
};

class Screen {
private:
    deque<Surface> m_surfaces;
    vector<string> m_screen;
    vector<string> m_lastScreen;
    vector<int> m_sortIndex;
    array<int, 2> m_size;
    ofstream m_file;
    int m_index{0};
    string m_defaultCharacter {" "};
    bool m_sizeChange {true};
    bool m_sorted {false};

    void m_getWinSize();
    void m_sortSurfaces();
    void m_initScreen();
    void m_render();

public:
    int amount;
    Screen();
    ~Screen();
    array<int, 2> getSize();
    Surface& append(array<int, 2> size, array<int, 2> offset, string ch = " ", int z = -1);
    void flip();
};



// Input
class Input{
private:
    bool is_char();


public:
    Input();
    ~Input();
    char get_char();
};

struct Node{
    int data = -1;
    int px;
    int py;
    int sx;
    int sy;
    Node* left;
    Node* right;
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

