#include "../tui.h"
#include <iostream>
#include <fstream>

using namespace std;


Label::Label(Surface& i_root, const string& i_text, array<int, 2> i_offset):root(i_root), offset(i_offset), str(i_text) {
    updateSurface();
}

void Label::updateSurface(){
    if (str != oldStr){
        m_parseString(str);
        m_getSize();
        // TODO Do i need a better z handling here
        surf = Surface(size, " ", 0, offset);
        string ch = "";
        for (int y = 0; y < text.size(); y++){

            InputString istr(text[y]);
            int x = 0;
            int index = 0;
            while (x <= istr.size()){
                ch = istr[index];
                index++;
                int width = istr.getWidth(ch);
                surf[y * size[0] + x].set_ch(ch);
                surf[y * size[0] + x].genrate();
                x++;
                for (int i = 2; i <= width; i++){
                    if (x <= istr.size()){
                        surf[y * size[0] + x].ch_def = false;
                        x++;
                    }
                }
            } 
        }
        root.blit(surf);
        oldStr = str;
    }
}

void Label::m_getSize(){
    size[1] = text.size();
    size[0] = 0;
    for (const string& line: text){
        if (line.size() > size[0]){
            size[0] = line.size();
        }
    }
}

void Label::m_parseString(const string& i_str){
    text.clear();
    string line;
    for (const char ch: i_str){
        if (ch == '\n'){
            text.push_back(line);
            line = "";
            continue;
        }
        line += ch;
    }
    text.push_back(line);
}



