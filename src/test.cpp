#include <iostream>
#include "tui.h"

using namespace std;
int main(){
    Screen screen;
    int size[2] {10, 10};
    int offset[2] {5, 5};
    auto &surf = screen.append(size, " ", 0, offset);
    surf.fill(255, 255, 255);
    cout << "\x1b[3J";
    while (true){
        screen.flip();
    }
}
