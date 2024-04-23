#include "sdlms/statusbar.hpp"
#include "wz/Property.hpp"
#include "util/wz_util.hpp"

StatusBar::StatusBar()
{
    auto StatusBar = util::WzUtil::current()->UI->get_root()->find_from_path(u"StatusBar.img");
    {
        auto _backgrnd = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(StatusBar->find_from_path(u"base/backgrnd")));

        _backgrnd._rect.y = Graphics::SCREEN_HEIGHT - _backgrnd._rect.h;

        for (size_t i = 0; i < Graphics::SCREEN_WIDTH; i += _backgrnd._rect.w)
        {
            SDL_FRect rect{(float)i, _backgrnd._rect.y, _backgrnd._rect.w, _backgrnd._rect.h};
            backgrnd.push_back(Sprite(_backgrnd._texture, rect, SDL_FLIP_NONE));
        }

        backgrnd2 = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(StatusBar->find_from_path(u"base/backgrnd2")));

        backgrnd2._rect.y = Graphics::SCREEN_HEIGHT - backgrnd2._rect.h;

        chatTarget = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(StatusBar->find_from_path(u"base/chatTarget")));

        chatTarget._rect.y = Graphics::SCREEN_HEIGHT - backgrnd2._rect.h + 8;

        quickSlot = Sprite::load_sprite(dynamic_cast<wz::Property<wz::WzCanvas> *>(StatusBar->find_from_path(u"base/quickSlot")));

        quickSlot._rect.x = Graphics::SCREEN_WIDTH - quickSlot._rect.w;
        quickSlot._rect.y = Graphics::SCREEN_HEIGHT - quickSlot._rect.h;
    }
    {
        // load_button
        auto _BtShop = StatusBar->find_from_path(u"BtShop");
        BtShop = std::move(Button(EventSprite::load_event_sprite(Button::EventMap, _BtShop, 590, Graphics::SCREEN_HEIGHT - 18), _BtShop->path));

        auto _BtMenu = StatusBar->find_from_path(u"BtMenu");
        BtMenu = Button(EventSprite::load_event_sprite(Button::EventMap, _BtMenu, 635, Graphics::SCREEN_HEIGHT - 18), _BtMenu->path);

        auto _BtChat = StatusBar->find_from_path(u"BtChat");
        BtChat = Button(EventSprite::load_event_sprite(Button::EventMap, _BtChat, 680, Graphics::SCREEN_HEIGHT - 18), _BtChat->path);

        auto _BtNPT = StatusBar->find_from_path(u"BtNPT");
        BtNPT = Button(EventSprite::load_event_sprite(Button::EventMap, _BtNPT, 725, Graphics::SCREEN_HEIGHT - 18), _BtNPT->path);
    }
}

void StatusBar::draw()
{
    for (auto &it : backgrnd)
    {
        it.draw_static();
    }
    backgrnd2.draw_static();
    chatTarget.draw_static();
    quickSlot.draw_static();

    BtShop.draw_static();
    BtMenu.draw_static();
    BtChat.draw_static();
    BtNPT.draw_static();
}