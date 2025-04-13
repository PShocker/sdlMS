#include "UINotice.h"
#include "Resources/Wz.h"
#include "Button.h"
#include "Systems/UI.h"
#include "Commons/Commons.h"
#include "Systems/PlayerStateMachine.h"

void UINotice::show()
{
    UINotice::x = Camera::w / 2;
    UINotice::y = Camera::h / 2;

    auto ui_node = Wz::UI->get_root();
    backgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui_node->find_from_path(u"UIWindow.img/Notice/0")));

    Button::load(u"Basic.img/BtOK2", BtOK2);

    ui_index.push_back(UIIndex::UI_UINotice);
    UINotice::open = true;
}

void UINotice::hide()
{
    ui_index.remove(UIIndex::UI_UINotice);
    UINotice::open = false;
}

void UINotice::over()
{
    for (auto &[key, val] : UINotice::position_map)
    {
        auto rect = val;
        rect.x += UINotice::x;
        rect.y += UINotice::y;
        Button::over(rect, *key);
    }
}

bool UINotice::mousein()
{
    SDL_FPoint point = {Cursor::x, Cursor::y};
    SDL_FRect rect;
    rect.x = UINotice::x;
    rect.y = UINotice::y;
    rect.w = UINotice::backgrnd->w;
    rect.h = UINotice::backgrnd->h;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        return true;
    }
    return false;
}

void UINotice::click()
{
    for (auto &[key, val] : UINotice::position_map)
    {
        auto rect = val;
        rect.x += UINotice::x;
        rect.y += UINotice::y;
        Button::click(rect, *key, UINotice::click_map);
    }
}

void UINotice::BtOK2_func()
{
    Player::hp = Player::max_hp;
    Player::mp = Player::max_mp;
    World::TransPort::id = Map::load_returnmap(Map::id);
    World::TransPort::tn = u"sp";
    hide();
}