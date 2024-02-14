#include "util/statusbar_util.hpp"
#include "util/wz_util.hpp"

#include "sdlms/graphics.hpp"

#include "wz/Property.hpp"

#include "sdlms/graphics.hpp"

namespace util
{
    StatusBarUtil::StatusBarUtil()
    {
        _renderer = Graphics::current()->getRenderer();
        _sprite_util = SpriteUtil::current();
        _freetype_util = FreeTypeUtil::current();
        _string_util = StringUtil::current();

        _ui_node = WzUtil::current()->UI->get_root();
    }


    std::vector<Sprite> StatusBarUtil::load_statusbar()
    {
        std::vector<Sprite> v_s;

        auto ui = _ui_node->find_from_path(u"StatusBar.img");

        auto backgrnd = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui->find_from_path(u"base/backgrnd")));

        backgrnd._rect.y = Graphics::SCREEN_HEIGHT - backgrnd._rect.h;

        for (size_t i = 0; i < Graphics::SCREEN_WIDTH; i += backgrnd._rect.w)
        {
            SDL_FRect rect{i, backgrnd._rect.y, backgrnd._rect.w, backgrnd._rect.h};
            v_s.push_back(Sprite(backgrnd._texture, rect, SDL_FLIP_NONE));
        }

        auto backgrnd2 = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui->find_from_path(u"base/backgrnd2")));

        backgrnd2._rect.y = Graphics::SCREEN_HEIGHT - backgrnd2._rect.h;

        v_s.push_back(backgrnd2);

        auto chatTarget = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui->find_from_path(u"base/chatTarget")));

        chatTarget._rect.y = Graphics::SCREEN_HEIGHT - backgrnd2._rect.h + 8;

        v_s.push_back(chatTarget);

        auto quickSlot = _sprite_util->load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(ui->find_from_path(u"base/quickSlot")));

        quickSlot._rect.x = Graphics::SCREEN_WIDTH - quickSlot._rect.w;
        quickSlot._rect.y = Graphics::SCREEN_HEIGHT - quickSlot._rect.h;

        v_s.push_back(quickSlot);

        return v_s;
    }

    std::vector<EventSprite *> StatusBarUtil::load_event_sprite()
    {
        std::vector<EventSprite *> v_s;

        auto StatusBar = _ui_node->find_from_path(u"StatusBar.img");

        auto BtShop = _sprite_util->load_event_sprite(StatusBar->find_from_path(u"BtShop"), 590, Graphics::SCREEN_HEIGHT - 18);

        v_s.push_back(BtShop);

        auto BtMenu = _sprite_util->load_event_sprite(StatusBar->find_from_path(u"BtMenu"), 635, Graphics::SCREEN_HEIGHT - 18);

        v_s.push_back(BtMenu);

        auto BtChat = _sprite_util->load_event_sprite(StatusBar->find_from_path(u"BtChat"), 680, Graphics::SCREEN_HEIGHT - 18);

        v_s.push_back(BtChat);

        auto BtNPT = _sprite_util->load_event_sprite(StatusBar->find_from_path(u"BtNPT"), 725, Graphics::SCREEN_HEIGHT - 18);

        v_s.push_back(BtNPT);
        return v_s;
    }
}
