#include "../tui.h"
#include <utf8proc.h>
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
size_t InputString::size(){
    return str.size();
}
string InputString::operator[](int i){
    return str.at(i);
    
}

int InputString::getWidth(string s){
    if (s.empty()) return 0;
    utf8proc_int32_t cp = 0;
    // utf8proc_iterate decodes the first codepoint from UTF-8 bytes.
    // It returns number of bytes consumed, or a negative value on error.
    int consumed = utf8proc_iterate(
        reinterpret_cast<const utf8proc_uint8_t*>(s.data()),
        static_cast<utf8proc_ssize_t>(s.size()),
        &cp
    );

    if (consumed < 0) return 1;                 // invalid UTF-8
    if (static_cast<size_t>(consumed) != s.size()) {
        return 1;
    }

    int w = utf8proc_charwidth(cp);             // 0/1/2 (and sometimes -1)
    if (w < 0) return 1;                        // non-printable -> treat as 1
    return w;
}

InputString::InputString(string s){
    int index = 0;
    while (index < s.size()){
        char ch {s[index]};
        int len = get_byte_len(ch);
        if (len != -1){
            string ch {""};
            int new_index = index + len;
            for (int i = index; i < new_index; i++){
                ch += s[i];
            }
            str.push_back(ch);
            index = new_index;
        }
        
    }

}
