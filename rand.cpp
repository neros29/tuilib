#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

int main(){
    char ch[8]{"😀"};
    string ESC {"\x1B"};
    ostringstream oss;
    ostringstream _fg;
    ostringstream _bg;
    _fg << ESC << "[38;2;"<< 255 << ";" << 255 << ";"<< 255 <<"m";
    _bg << ESC << "[48;2;"<< 255 << ";" << 255 << ";" << 255 << "m";
    oss << _bg.str() << _fg.str() << ch << ESC << "[0m";
    string ansii = oss.str();
    char ch[ansii.size()];
    int index {0};
    for(char i:ansii){
        ch[index] = i;
        index++;
    }
    write(1, ch, ansii.size());
}
