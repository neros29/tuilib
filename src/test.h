#include <iostream>
#include "core/tui.h"
using namespace std;
inline void testChColorError(){
    Character ch;
    
    try{
        ch.set_bg(256, 257, 258);
        cout << "❌ Test Character color error faild" << endl;
    }
    catch(invalid_argument){
        cout << "✅ Test complteted succesfuly" << endl;

    }
}
inline void testChCharacterError(){
    Character ch;
    try{
        ch.set_ch("");
        cout << "❌ Test Character charecter error faild" << endl;
    }
    catch(invalid_argument){
        cout << "✅ Test complteted succesfuly" << endl;

    }
}


