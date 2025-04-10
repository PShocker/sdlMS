#include "GainTip.h"
#include "Cursor.h"
#include "Resources/Wz.h"
#include "Core/Core.h"

void GainTip::run()
{
    for (auto it = GainTip::gaintips.begin(); it != GainTip::gaintips.end();)
    {
        if (it->destory <= Window::dt_now)
        {
            SDL_DestroyTexture(it->texture);
            it = GainTip::gaintips.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void GainTip::push(const std::u16string &index, int num)
{
    auto str = std::to_string(num);
    str = "(+" + str + ")";
    auto num_str = std::u16string{str.begin(), str.end()};
    if (index == u"e")
    {
        auto str = u"\u5f97\u5230\u7ecf\u9a8c\u503c" + num_str;
        auto texture = FreeType::load(str, SDL_Color{255, 255, 255, 255}, 0, 12);
        gaintips.push_back({texture, Window::dt_now + 5000});
    }
    else
    {
    }
}
