#pragma once
#include <vector>
#include <string>
#include <deque>


using namespace std;

class Character{
private:
    string ch = "";
    bool bg_def = true;
    bool fg_def = true;
    int fg[3];
    int bg[3];
public:
    string ansii {""};
    void set_fg(int r, int g, int b);
    void set_bg(int r, int g, int b);
    void set_ch(string chi);
    void genrate();
};

class Surface{
public:
    int size[2];
    vector <Character> surface;
    int r_cords[2];
    int z {0};
    Character operator[](int x);
    Surface(int size[2], string ch, int z, int offsett[2]);
    void fill(int r, int g, int b);
    
};

class Screen{
private:
    deque <Surface> surfaces;
    vector <string> screen;
    vector <int> sort_idx;
    int idx {0};
    string def_chr = " ";
    bool soreted {false};
    bool size_ch {true};
    void get_win_size();
    void sort_surfaces();
    void init_screen();
    void render();

public:
    int size[0];
    Surface &append(int size[2], string ch, int z, int offset[2]);
    void flip();
};
