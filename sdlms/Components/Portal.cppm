module;
#include <string>
export module components:portal;

export struct Portal
{
    int tm = 0;
    int pt = 0;
    std::u16string pn;
    std::u16string tn;
    Portal() = default;
};
