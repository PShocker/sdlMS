module;

#include <string>

export module components:weaponinfo;

export struct WeaponInfo
{
    std::u16string afterImage;
    std::u16string sfx;
    bool stand1 = false;
    bool walk1 = false;
    WeaponInfo(const std::u16string &id);
};
