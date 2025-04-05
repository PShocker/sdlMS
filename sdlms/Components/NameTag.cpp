#include "NameTag.h"
#include "Core/FreeType.h"

void NameTag::push(NameTag *nametag, std::u16string str, SDL_Color color, std::u16string type)
{
    auto str_texture = FreeType::load(str, color, 0, 12);
    nametag->nametags.push_back({str_texture, nullptr});
}
