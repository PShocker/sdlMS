module;

#include <string>

export module components:weaponinfo;

export struct WeaponInfo
{
    std::u16string afterImage;
    std::u16string sfx;
    bool stand1;
    bool walk1;
    WeaponInfo(const std::u16string &id);
};
