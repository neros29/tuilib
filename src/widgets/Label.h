#include "../api.h"
class Label{
private:
    Surface& root;
    Surface surf;
    const string& str;
    string oldStr;
    vector<string> text;
    array<int, 2> size;
    array<int, 2> offset;
    void m_getSize();
    void m_parseString(const string& i_str);
public:
    Label(Surface& i_root, const string& i_text, array<int, 2> i_offset);
    void updateSurface();
};

