#include "../api.h"
class Label{
private:
    Surface& root;
    Surface surf;
    const std::string& str;
    std::string oldStr;
    std::vector<std::string> text;
    std::array<int, 2> size;
    std::array<int, 2> offset;
    void m_getSize();
    void m_parseString(const std::string& i_str);
public:
    Label(Surface& i_root, const std::string& i_text, std::array<int, 2> i_offset);
    void updateSurface();
};

