#include "../tui.h"
#include <iostream>

using namespace std;

int InputString::get_byte_len(unsigned char firstByte) {
    // Check the value of the first byte to determine the sequence length
    if (firstByte >= 0x00 && firstByte <= 0x7F) {
        // 0xxxxxxx: 1 byte
        return 1;
    } else if (firstByte >= 0xC2 && firstByte <= 0xDF) {
        // 110xxxxx: 2 bytes
        return 2;
    } else if (firstByte >= 0xE0 && firstByte <= 0xEF) {
        // 1110xxxx: 3 bytes
        return 3;
    } else if (firstByte >= 0xF0 && firstByte <= 0xF7) {
        // 11110xxx: 4 bytes
        return 4;
    } else if (firstByte >= 0xF8 && firstByte <= 0xFB) {
        // 111110xx: 5 bytes
        return 5;
    } else if (firstByte == 0xFC) {
        // 1111110x: 6 bytes
        return 6;
    } 
    else {
        // Invalid first byte - not a valid sequence
        return -1; // Or throw an exception, depending on desired behavior
    }
}
auto InputString::begin(){
    return str.begin();
}
auto InputString::end(){
    return str.end();
}
size_t InputString::size(){
    return str.size();
}
string InputString::operator[](int i){
    return str[i];
    
}

InputString::InputString(string s){
    int index = 0;
    while (index < s.size()){
        char ch {s[index]};
        int len = get_byte_len(ch);
        if (len != -1){
            string ch {""};
            int new_index = index + len;
            for (int i = index; i <= new_index; i++){
                ch += s[i];
            }
            str.push_back(ch);
            index = new_index;
        }
        
    }

}
