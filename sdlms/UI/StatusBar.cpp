#include "StatusBar.h"
#include "KeyConfig.h"
#include "UIItem.h"
#include "UISkill.h"
#include "UIStat.h"
#include "UIEquip.h"
#include "MiniMap.h"
#include "QuickSlot.h"
#include "Button.h"
#include "wz/Property.hpp"
#include "Resources/Wz.h"
#include "Components/Components.h"

void StatusBar::run()
{
    load_hp();
    load_mp();
    load_exp();
}

void StatusBar::over()
{
    for (auto &[key, val] : StatusBar::position_map)
    {
        auto rect = val;
        rect.y += Camera::h;
        Button::over(rect, *key);
    }
}

void StatusBar::load()
{
    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd")));
    backgrnd2 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/backgrnd2")));
    chatTarget = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/chatTarget")));
    iconMemo = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/iconMemo")));
    box = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/box")));
    chat = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/base/chat")));

    graduation = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/graduation")));
    bar = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/bar")));
    gray = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"StatusBar.img/gauge/gray")));

    Button::load(u"StatusBar.img/BtShop", BtShop);
    Button::load(u"StatusBar.img/BtChat", BtChat);
    Button::load(u"StatusBar.img/BtNPT", BtNPT);
    Button::load(u"StatusBar.img/BtMenu", BtMenu);
    Button::load(u"StatusBar.img/BtShort", BtShort);
    Button::load(u"StatusBar.img/BtClaim", BtClaim);
    Button::load(u"StatusBar.img/EquipKey", EquipKey);
    Button::load(u"StatusBar.img/InvenKey", InvenKey);
    Button::load(u"StatusBar.img/StatKey", StatKey);
    Button::load(u"StatusBar.img/SkillKey", SkillKey);
    Button::load(u"StatusBar.img/KeySet", KeySet);
    Button::load(u"StatusBar.img/QuickSlot", QuickSlot);
    Button::load(u"StatusBar.img/QuickSlotD", QuickSlotD);
    Button::load(u"Basic.img/BtMax2", BtChatMax);

    auto node = ui_node->find_from_path(u"Basic.img/LevelNo");
    for (int i = 0; i < 10; i++)
    {
        LevelNo[i] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(std::to_string(i))));
    }

    name = FreeType::load(Player::name, SDL_Color{255, 255, 255, 255}, 0, 12);

    job = FreeType::load(u"全职业精通", SDL_Color{255, 255, 255, 255}, 0, 12);

    node = ui_node->find_from_path(u"StatusBar.img/number");
    for (int i = 0; i < 10; i++)
    {
        number[i] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(std::to_string(i))));
    }
    number[10] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"Lbracket")));
    number[11] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"Rbracket")));
    number[12] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"slash")));
    number[13] = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->get_child(u"percent")));
}

void StatusBar::init()
{
    load();
    load_backgrnd3();
    load_level();
    load_job();
    load_name();
    load_hp();
    load_bar_fade(322, (float)Player::hp / Player::max_hp, 106);
    load_mp();
    load_bar_fade(431, (float)Player::mp / Player::max_mp, 107);
    load_exp();
    load_bar_fade(554, (float)Player::exp / Player::max_exp, 117);
    load_vscr();
}

void StatusBar::click()
{
    for (auto &[key, val] : StatusBar::position_map)
    {
        auto rect = val;
        rect.y += Camera::h;
        Button::click(rect, *key, StatusBar::click_map);
    }
}

void StatusBar::load_backgrnd3()
{
    // 提取渲染好背景backgrnd3
    if (backgrnd3)
    {
        SDL_DestroyTexture(backgrnd3);
    }
    backgrnd3 = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB4444, Camera::w, backgrnd->h);
    SDL_SetRenderTarget(Window::renderer, backgrnd3);
    // 渲染backgrnd
    SDL_FRect pos_rect{(float)0, (float)0, (float)StatusBar::backgrnd->w, (float)StatusBar::backgrnd->h};
    for (float i = 0; i <= Camera::w; i += StatusBar::backgrnd->w)
    {
        pos_rect.x = i;
        SDL_RenderTexture(Window::renderer, backgrnd, nullptr, &pos_rect);
    }
    pos_rect = SDL_FRect{(float)0, (float)0, (float)StatusBar::backgrnd2->w, (float)StatusBar::backgrnd2->h};
    SDL_RenderTexture(Window::renderer, StatusBar::backgrnd2, nullptr, &pos_rect);

    auto src_rect = SDL_FRect{(float)21, (float)0, (float)21, (float)19};
    pos_rect = SDL_FRect{(float)592, (float)7, (float)21, (float)19};
    SDL_RenderTexture(Window::renderer, StatusBar::box, &src_rect, &pos_rect);

    pos_rect = SDL_FRect{(float)596, (float)11, (float)StatusBar::iconMemo->w, (float)StatusBar::iconMemo->h};
    SDL_RenderTexture(Window::renderer, StatusBar::iconMemo, nullptr, &pos_rect);

    pos_rect = SDL_FRect{(float)215, (float)37, (float)StatusBar::bar->w, (float)StatusBar::bar->h};
    SDL_RenderTexture(Window::renderer, StatusBar::bar, nullptr, &pos_rect);
    SDL_RenderTexture(Window::renderer, StatusBar::graduation, nullptr, &pos_rect);

    pos_rect = SDL_FRect{(float)0, (float)6, (float)566, (float)24};
    SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(Window::renderer, 136, 136, 136, 255);
    SDL_RenderFillRect(Window::renderer, &pos_rect);

    SDL_SetRenderTarget(Window::renderer, nullptr);
}

void StatusBar::load_level()
{
    SDL_SetRenderTarget(Window::renderer, backgrnd3);
    // 渲染等级
    auto l = Player::level;
    auto length = static_cast<int>(std::floor(std::log10(l)) + 1);
    SDL_FRect pos_rect{(float)22 + length * 13, (float)47, (float)11, (float)13};
    // 分解各位数字
    while (l > 0)
    {
        int digit = l % 10;
        SDL_RenderTexture(Window::renderer, StatusBar::LevelNo[digit], nullptr, &pos_rect);
        pos_rect.x -= 13;
        l /= 10;
    }
    SDL_SetRenderTarget(Window::renderer, nullptr);
}

void StatusBar::load_job()
{
    SDL_SetRenderTarget(Window::renderer, backgrnd3);

    SDL_FRect pos_rect{(float)87, (float)37, (float)StatusBar::job->w, (float)StatusBar::job->h};
    SDL_RenderTexture(Window::renderer, StatusBar::job, nullptr, &pos_rect);

    SDL_SetRenderTarget(Window::renderer, nullptr);
}

void StatusBar::load_name()
{
    SDL_SetRenderTarget(Window::renderer, backgrnd3);

    SDL_FRect pos_rect{(float)87, (float)StatusBar::job->h + 39, (float)StatusBar::name->w, (float)StatusBar::name->h};
    SDL_RenderTexture(Window::renderer, StatusBar::name, nullptr, &pos_rect);

    SDL_SetRenderTarget(Window::renderer, nullptr);
}

void StatusBar::load_bar()
{
    SDL_FRect pos_rect{(float)215, (float)37, (float)StatusBar::bar->w, (float)StatusBar::bar->h};
    SDL_RenderTexture(Window::renderer, StatusBar::bar, nullptr, &pos_rect);
}

void StatusBar::load_bar_digit(int x, int cur, int max)
{
    SDL_SetRenderTarget(Window::renderer, backgrnd3);
    SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 0);

    auto w = 88;
    auto h = 9;
    SDL_FRect pos_rect{(float)x, (float)40, (float)w, (float)h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_FRect src_rect{(float)x, (float)40, (float)w, (float)h};
    SDL_RenderTexture(Window::renderer, StatusBar::backgrnd, &src_rect, &pos_rect);
    SDL_RenderTexture(Window::renderer, StatusBar::backgrnd2, &src_rect, &pos_rect);

    pos_rect = {(float)x, (float)40, (float)StatusBar::number[10]->w, (float)StatusBar::number[10]->h};
    SDL_RenderTexture(Window::renderer, StatusBar::number[10], nullptr, &pos_rect);

    const auto load_num = [](int x, int num)
    {
        auto length = static_cast<int>(std::floor(std::log10(num)) + 1);
        length = std::max(1, length);
        x += length * 6 - 2;
        SDL_FRect pos_rect{(float)x, (float)41, (float)StatusBar::number[0]->w, (float)StatusBar::number[0]->h};
        // 分解各位数字
        if (num == 0)
        {
            SDL_RenderTexture(Window::renderer, StatusBar::number[0], nullptr, &pos_rect);
        }
        else
        {
            while (num > 0)
            {
                int digit = num % 10;
                SDL_RenderTexture(Window::renderer, StatusBar::number[digit], nullptr, &pos_rect);
                pos_rect.x -= 6;
                num /= 10;
            }
        }
        return x;
    };
    x = load_num(x, cur);

    x += 7;
    pos_rect = SDL_FRect{(float)x, (float)41, (float)StatusBar::number[12]->w, (float)StatusBar::number[12]->h};
    SDL_RenderTexture(Window::renderer, StatusBar::number[12], nullptr, &pos_rect);
    x += 4;
    x = load_num(x, max);
    x += 7;
    pos_rect = SDL_FRect{(float)x, (float)40, (float)StatusBar::number[11]->w, (float)StatusBar::number[11]->h};
    SDL_RenderTexture(Window::renderer, StatusBar::number[11], nullptr, &pos_rect);
    SDL_SetRenderTarget(Window::renderer, nullptr);
}

void StatusBar::load_bar_fade(float x, float percent, int w)
{
    SDL_SetRenderTarget(Window::renderer, backgrnd3);
    SDL_SetRenderDrawBlendMode(Window::renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(Window::renderer, 0, 0, 0, 0);

    auto h = 16;
    SDL_FRect pos_rect{(float)x - w, (float)52, (float)w, (float)h};
    SDL_RenderFillRect(Window::renderer, &pos_rect);
    SDL_FRect src_rect{(float)x - w, (float)52, (float)w, (float)h};
    SDL_RenderTexture(Window::renderer, StatusBar::backgrnd, &src_rect, &pos_rect);
    SDL_RenderTexture(Window::renderer, StatusBar::backgrnd2, &src_rect, &pos_rect);
    src_rect = {(float)x - 215 - w, (float)15, (float)w, (float)h};
    SDL_RenderTexture(Window::renderer, StatusBar::bar, &src_rect, &pos_rect);
    auto length = (1 - percent) * w;
    pos_rect = {(float)x - length, (float)52, (float)length, (float)StatusBar::gray->h};
    SDL_RenderTexture(Window::renderer, StatusBar::gray, nullptr, &pos_rect);
    pos_rect = {(float)x - w, (float)52, (float)w, (float)h};
    SDL_RenderTexture(Window::renderer, StatusBar::graduation, &src_rect, &pos_rect);

    SDL_SetRenderTarget(Window::renderer, nullptr);
}

void StatusBar::load_hp()
{
    static int last_hp;
    static int last_max_hp;
    static float last_percent = (float)Player::hp / Player::max_hp;
    if (last_hp != Player::hp || last_max_hp != Player::max_hp)
    {
        load_bar_digit(236, Player::hp, Player::max_hp);
        last_hp = Player::hp;
        last_max_hp = Player::max_hp;
    }
    if ((float)Player::hp / Player::max_hp < last_percent)
    {
        last_percent = std::max(last_percent - (float)Window::delta_time / 2000, (float)Player::hp / Player::max_hp);
    }
    else if ((float)Player::hp / Player::max_hp > last_percent)
    {
        last_percent = std::min(last_percent + (float)Window::delta_time / 2000, (float)Player::hp / Player::max_hp);
    }
    if (last_percent != (float)Player::hp / Player::max_hp)
    {
        load_bar_fade(322, last_percent, 106);
    }
    return;
}

void StatusBar::load_mp()
{
    static int last_mp;
    static int last_max_mp;
    static float last_percent = (float)Player::mp / Player::max_mp;
    if (last_mp != Player::mp || last_max_mp != Player::max_mp)
    {
        load_bar_digit(347, Player::mp, Player::max_mp);
        last_mp = Player::mp;
        last_max_mp = Player::max_mp;
    }
    if ((float)Player::mp / Player::max_mp < last_percent)
    {
        last_percent = std::max(last_percent - (float)Window::delta_time / 2000, (float)Player::mp / Player::max_mp);
    }
    else if ((float)Player::mp / Player::max_mp > last_percent)
    {
        last_percent = std::min(last_percent + (float)Window::delta_time / 2000, (float)Player::mp / Player::max_mp);
    }
    if (last_percent != (float)Player::mp / Player::max_mp)
    {
        load_bar_fade(431, last_percent, 107);
    }
    return;
}

void StatusBar::load_exp()
{
    static int last_exp;
    static int last_max_exp;
    static float last_percent = (float)Player::exp / Player::max_exp;
    if (last_exp != Player::exp || last_max_exp != Player::max_exp)
    {
        load_bar_digit(464, Player::exp, Player::max_exp);
        last_exp = Player::exp;
        last_max_exp = Player::max_exp;
    }
    if ((float)Player::exp / Player::max_exp < last_percent)
    {
        last_percent = std::max(last_percent - (float)Window::delta_time / 2000, (float)Player::exp / Player::max_exp);
    }
    else if ((float)Player::exp / Player::max_exp > last_percent)
    {
        last_percent = std::min(last_percent + (float)Window::delta_time / 2000, (float)Player::exp / Player::max_exp);
    }
    if (last_percent != (float)Player::exp / Player::max_exp)
    {
        load_bar_fade(554, last_percent, 117);
    }
    return;
}

void StatusBar::load_vscr()
{
    auto ui_node = Wz::UI->get_root();
    if (!chatOpen)
    {
        auto prev = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"Basic.img/VScr5/disabled/prev")));
        auto next = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"Basic.img/VScr5/disabled/next")));
        SDL_SetRenderTarget(Window::renderer, backgrnd3);
        SDL_FRect pos_rect{(float)551, (float)5, (float)prev->w, (float)prev->h};
        SDL_RenderTexture(Window::renderer, prev, nullptr, &pos_rect);
        pos_rect = SDL_FRect{(float)551, (float)5 + prev->h, (float)next->w, (float)next->h};
        SDL_RenderTexture(Window::renderer, next, nullptr, &pos_rect);
        SDL_SetRenderTarget(Window::renderer, nullptr);
    }
    else
    {
    }
    if (chats.empty())
    {
        chats.push_back(FreeType::load(u"欢迎来到冒险岛,现在开始你的旅程吧!", SDL_Color{255, 255, 255, 255}, 0, 12));
    }
}

void StatusBar::QuickSlot_func()
{
    StatusBar::alpha += 20;
    StatusBar::alpha = StatusBar::alpha > 255 ? 55 : StatusBar::alpha;
    KeyConfig::alpha = StatusBar::alpha;
    UIItem::alpha = StatusBar::alpha;
    MiniMap::alpha = StatusBar::alpha;
    QuickSlot::alpha = StatusBar::alpha;
}

void StatusBar::QuickSlotD_func()
{
    StatusBar::alpha -= 20;
    StatusBar::alpha = StatusBar::alpha < 55 ? 255 : StatusBar::alpha;
    KeyConfig::alpha = StatusBar::alpha;
    UIItem::alpha = StatusBar::alpha;
    MiniMap::alpha = StatusBar::alpha;
    QuickSlot::alpha = StatusBar::alpha;
}

void StatusBar::KeySet_func()
{
    KeyConfig::show();
}

void StatusBar::InvenKey_func()
{
    UIItem::show();
}

void StatusBar::SkillKey_func()
{
    UISkill::show();
}

void StatusBar::StatKey_func()
{
    UIStat::show();
}

void StatusBar::EquipKey_func()
{
    UIEquip::show();
}
