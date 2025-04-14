#include "UIItem.h"
#include "Resources/Wz.h"
#include "Button.h"
#include "Systems/UI.h"

void UIItem::run()
{
}

void UIItem::show()
{
    UIItem::x = Camera::w / 2;
    UIItem::y = Camera::h / 2;

    FullBackgrnd = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(Wz::UI->get_root()->find_from_path(u"UIWindow.img/Item/FullBackgrnd")));
    load_tab();

    Button::load(u"Basic.img/BtClose", BtClose);

    ui_index.push_back(UIIndex::UI_UIItem);
    UIItem::open = true;
}

void UIItem::hide()
{
    ui_index.remove(UIIndex::UI_UIItem);
    UIItem::open = false;
}

void UIItem::over()
{
    for (auto &[key, val] : UIItem::position_map)
    {
        auto rect = val;
        rect.x += UIItem::x;
        rect.y += UIItem::y;
        Button::over(rect, *key);
    }
}

bool UIItem::mousein()
{
    SDL_FPoint point = {Cursor::x, Cursor::y};
    SDL_FRect rect;
    rect.x = UIItem::x;
    rect.y = UIItem::y;
    rect.w = UIItem::FullBackgrnd->w;
    rect.h = UIItem::FullBackgrnd->h;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        return true;
    }
    return false;
}

void UIItem::click()
{
    for (auto &[key, val] : UIItem::position_map)
    {
        auto rect = val;
        rect.x += UIItem::x;
        rect.y += UIItem::y;
        Button::click(rect, *key, UIItem::click_map);
    }
    click_tab();
}

void UIItem::click_tab()
{
    SDL_FPoint point = {Cursor::x, Cursor::y};
    SDL_FRect rect;
    rect.x = UIItem::x + 4;
    rect.y = UIItem::y + 23;
    rect.w = UIItem::tabs[0]->w * 5;
    rect.h = UIItem::tabs[0]->h;
    if (SDL_PointInRectFloat(&point, &rect))
    {
        UIItem::active_tab = (Cursor::x - rect.x) / UIItem::tabs[0]->w;
    }
}

void UIItem::load_tab()
{
    if (UIItem::tabs.empty())
    {
        const auto render_tab = [](SDL_Texture *l, SDL_Texture *f, SDL_Texture *r, SDL_Texture *title, SDL_Texture *target)
        {
            SDL_SetRenderTarget(Window::renderer, target);
            SDL_FRect pos_rect;
            pos_rect.x = 0;
            pos_rect.y = 0;
            pos_rect.w = l->w;
            pos_rect.h = l->h;
            SDL_RenderTexture(Window::renderer, l, nullptr, &pos_rect);
            pos_rect.x = l->w;
            pos_rect.y = 0;
            pos_rect.w = target->w - l->w - r->w;
            pos_rect.h = f->h;
            SDL_RenderTexture(Window::renderer, f, nullptr, &pos_rect);
            pos_rect.x = target->w - r->w;
            pos_rect.y = 0;
            pos_rect.w = r->w;
            pos_rect.h = r->h;
            SDL_RenderTexture(Window::renderer, r, nullptr, &pos_rect);
            pos_rect.x = target->w / 2 - title->w / 2;
            pos_rect.y = target->h / 2 - title->h / 2;
            pos_rect.w = title->w;
            pos_rect.h = title->h;
            SDL_RenderTexture(Window::renderer, title, nullptr, &pos_rect);
            SDL_SetRenderTarget(Window::renderer, nullptr);
        };

        auto node = Wz::UI->get_root()->find_from_path(u"Basic.img/Tab2");
        auto l0 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"left0")));
        auto l1 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"left1")));
        auto r0 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"right0")));
        auto r1 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"right1")));
        auto f0 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"fill0")));
        auto f1 = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(node->find_from_path(u"fill1")));

        node = Wz::UI->get_root()->find_from_path(u"UIWindow.img/Item/Tab/enabled");
        for (int i = 0; i <= 4; i++)
        {
            auto it = node->get_child(std::to_string(i));
            auto title = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(it));
            auto texture = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB4444,
                                                       44,
                                                       20);
            render_tab(l1, f1, r1, title, texture);
            tabs.push_back(texture);
        }
        node = Wz::UI->get_root()->find_from_path(u"UIWindow.img/Item/Tab/disabled");
        for (int i = 0; i <= 4; i++)
        {
            auto it = node->get_child(std::to_string(i));
            auto title = Texture::load(dynamic_cast<wz::Property<wz::WzCanvas> *>(it));
            auto texture = Texture::createBlankTexture(SDL_PIXELFORMAT_ARGB4444,
                                                       44,
                                                       20);
            render_tab(l0, f0, r0, title, texture);
            tabs.push_back(texture);
        }
    }
}

// 用来判断背包是否有空位来加载物品，并返回原物品序号
std::pair<int, int> UIItem::full(std::u16string id)
{
    static std::unordered_map<std::u16string, std::pair<uint8_t, uint8_t>> cache;
    std::pair<int, int> r = {-1, -1};
    if (id == u"09000000")
    {
        return {0, 0};
    }
    auto i = Item::load_item_index(id);
    if (i == 0 || !cache.contains(id))
    {
        for (int index = 0; index < 96; index++)
        {
            if (items[i][index].sprw == nullptr)
            {
                r = {i, index};
                break;
            }
        }
    }
    else
    {
        r = cache[id];
    }
    cache[id] = r;
    return r;
}

void UIItem::push(std::u16string id, int num)
{
    if (id != u"09000000")
    {
        auto [i, index] = full(id);
        items[i][index].id = id;
        items[i][index].num += num;
        items[i][index].sprw = Sprite::Wrap::load(Item::load(id)->find_from_path(u"info/icon"));
    }
    else
    {
    }
}

void UIItem::BtClose_func()
{
    UIItem::hide();
}
