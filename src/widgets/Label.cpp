#include "../api.h"
#include "../core/utils/logging.h"
#include "Label.h"

using namespace std;


Label::Label(Surface& i_root, const string& i_text, array<int, 2> i_offset):root(i_root), offset(i_offset), str(i_text) {
    updateSurface();
}
void Label::updateSurface(){
    if (str != oldStr){
        m_parseString(str);
        m_getSize();
        for (int y = 0; y < text.size(); y ++){
            InputString line(text[y]);
            int x = 0;
            string buff;
            for (int i = 0; i < line.size(); i ++){
                string code_point = line[i];
                buff += code_point;
                if (i + 1 < line.size() && InputString::getWidth(line[i + 1]) < 1){
                    continue;
                }
                int total_width = InputString::getWidth(buff);
                root[(y + offset[1]) * root.get_size()[0] + x + offset[0]].set_ch(buff);
                buff.clear();
                x++;
                if (total_width > 1){
                    for (int j = 0; j < total_width - 1; j++){
                        root[(y + offset[1]) * root.get_size()[0] + x + offset[0]].set_ch("X");
                        root[(y + offset[1]) * root.get_size()[0] + x + offset[0]].or_flags(core::flags::PLACEHOLDER);
                        x++;
                    }
                }

            }
        }
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



