#include "sdlms/portal.hpp"
#include "util/wz_util.hpp"
#include "util/map_util.hpp"

Portal::Portal(std::variant<Sprite, AnimatedSprite> dynamicsprite,
               int type, int tm,
               std::u16string url) : DynamicSprite(dynamicsprite),
                                     _type(type), _tm(tm),
                                     _url(url)
{
    _camera = Camera::current();

    _input = Input::current();
    // _input->event(std::bind(&event, this, std::placeholders::_1));
}

void Portal::draws()
{
    // 只绘制GAME类型的传送门
    for (auto &it : _portal)
    {
        if (it._type == Type::GAME)
        {
            it.draw();
        }
    }
}

void Portal::updates(int elapsedTime)
{
    for (auto &it : _portal)
    {
        it.update(elapsedTime);
    }
}
void Portal::event(const SDL_Event &event)
{
    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
    {
        if (_type == Type::GAME)
        {
            SDL_FPoint p{(float)event.button.x + _camera->_viewport.x, (float)event.button.y + _camera->_viewport.y};
            auto rf = rect();
            if (SDL_PointInFRect(&p, &rf))
            {
                printf("123Shocker");
            }
        }
    }
}

void Portal::load_portal(int mapId)
{
    auto node = util::MapUtil::current()->load_map_node(mapId);

    node = node->get_child(u"portal");

    auto _map_node = util::WzUtil::current()->Map->get_root();

    if (node != nullptr)
    {
        constexpr const char16_t *pt_list[] = {u"sp", u"pi", u"pv", u"pc", u"pg", u"tp", u"ps",
                                               u"pgi", u"psi", u"pcs", u"ph", u"psh", u"pcj",
                                               u"pci", u"pcig", u"pshg"};
        for (auto it : node->get_children())
        {
            if (it.second[0]->get_child(u"pt") != nullptr)
            {
                auto pt = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"pt"))->get();
                auto tm = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"tm"))->get();

                if (pt < 0 || pt >= sizeof(pt_list))
                {
                    continue;
                }
                else
                {
                    auto x = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"x"))->get();
                    auto y = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"y"))->get();
                    {
                        auto url = u"MapHelper.img/portal/editor/" + std::basic_string<char16_t>(pt_list[pt]);

                        auto pn = _map_node->find_from_path(url);

                        Sprite sprite = Sprite::load_sprite(pn, x, y);

                        Portal portal(sprite, Portal::Type::EDITOR, tm, url);

                        _portal.push_back(portal);
                    }

                    {
                        if (pt == 7)
                        {
                            pt = 2;
                        }
                        auto url = u"MapHelper.img/portal/game/" + std::basic_string<char16_t>(pt_list[pt]);
                        if (_map_node->find_from_path(url) != NULL)
                        {
                            if (_map_node->find_from_path(url + u"/default") != NULL)
                            {
                                // 三段式传送门
                            }
                            else
                            {
                                // 普通的传送门,通常为pv
                                auto animatedsprite = AnimatedSprite::load_animated_sprite(_map_node->find_from_path(url), x, y);

                                Portal portal(animatedsprite, Portal::Type::GAME, tm, url);
                                _portal.push_back(portal);
                            }
                        }
                    }
                }
            }
        }
    }
}
