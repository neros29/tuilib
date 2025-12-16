#include <iostream>
#include "src/tui.h"


using namespace std;
int main(){
    cout << "before" << endl;
    InputString str("😀 🎯 hello world");
    for (int i = 0; i< str.size(); i++){
        cout << str[i] << endl;
    }
}


