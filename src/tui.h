#pragma once
#include <deque>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

class InputString{
private:
    int get_byte_len(unsigned char firstbyte);
    vector<string> str;
public:
    auto begin();
    auto end();
    size_t size();
    InputString(string s);
    string operator[](int i);
};

class Character {
private:
    string ch = "";
    bool bg_def {true};
    bool fg_def {true};
    int fg[3];
    int bg[3];

public:
    bool dirty {true};
    string ansii{""};
    void set_fg(int r, int g, int b);
    void set_bg(int r, int g, int b);
    void set_ch(string chi);
    void genrate();
};

class Surface {

public:
    int size[2];
    vector<Character> surface;
    int r_cords[2];
    int z{0};
    Character& operator[](int x);
    Surface(int size[2], string ch, int z, int offsett[2]);
    void fill_bg(int r, int g, int b);
    void fill_fg(int r, int g, int b);
    void set_z(int z);
    void set_offset(int x, int y);
    void add_string(string str, int col_start, int row, int fg[3], int bg[3]);

};

class Screen {
private:
    deque<Surface> surfaces;
    vector<string> screen;
    vector<string> last_screen;
    vector<int> sort_idx;
    ofstream file;
    int idx{0};
    string def_chr = " ";
    bool size_ch{true};
    bool sorted{false};

    void get_win_size();
    void sort_surfaces();
    void init_screen();
    void render();

public:
    int size[2];
    int amount;
    Screen();
    ~Screen();
    vector<int> get_size();
    Surface& append(int size[2], string ch, int z, int offset[2]);
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
