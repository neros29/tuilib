#include <iostream>
#include "core/tui.h"
#include "api.h"

using namespace std;
inline void testChColorError(){
    Character ch;
    
    try{
        ch.set_bg(256, 257, 258);
        cout << "❌ Test " << __FUNCTION__ <<" Faild" <<  endl;
    }
    catch(invalid_argument){
        cout << "✅ Test complteted succesfuly" << endl;

    }
}
inline void testChCharacterError(){
    Character ch;
    try{
        ch.set_ch("");
        cout << "❌ Test " << __FUNCTION__ <<" Faild" <<  endl;
    }
    catch(invalid_argument){
        cout << "✅ Test complteted succesfuly" << endl;

    }
}

inline void testSurfSizeError(){
    try{
        InternalSurface surf({-1, -50}, " ", -10, {8, 4}, {"Up"});
        cout << "❌ Test " << __FUNCTION__ <<" Faild" <<  endl;
    }
    catch(invalid_argument){
        cout << "✅ Test complteted succesfuly" << endl;

    }
}

inline void testSurfChError(){
    try{
        InternalSurface surf({1, 50}, "", -10, {8, 4}, {"Up"});
        cout << "❌ Test " << __FUNCTION__ <<" Faild" <<  endl;
    }
    catch(invalid_argument){
        cout << "✅ Test complteted succesfuly" << endl;

    }
}
inline void testSurfValidKeysError(){
    std::vector<std::string> keys = {"Up"};
    InternalSurface tsurf({1, 50}, " ", -10, {8, 4}, keys);

    Surface surf(tsurf);
    try{
        surf.register_keys({"NotAValidKey"});
        cout << "❌ Test " << __FUNCTION__ <<" Faild" <<  endl;
    }
    catch(invalid_argument){
        cout << "✅ Test complteted succesfuly" << endl;

    }
}
