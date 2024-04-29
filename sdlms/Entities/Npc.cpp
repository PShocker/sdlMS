#include "Npc.h"
#include "Components/Sprite.h"
#include "Components/Transform.h"
#include "Resource/Wz.h"

Npc::Npc(wz::Node *node, std::u16string ts, int layer, World *world)
{
    if (node != nullptr)
    {
        // auto id_pos = npc_id.find_first_not_of(u'0');
        // // 去掉npc_id中前缀的0
        // npc_id = npc_id.substr(id_pos);
        // auto npc_info = util::StringUtil::current()->load_npc_info(npc_id);
        // auto name = npc_info[u"name"];
        // auto func = npc_info[u"func"];
        // auto eventsprite = EventSprite::load_event_sprite(Npc::EventMap, npc_node, x, y);

        // auto npc = Npc(eventsprite, npc_id, fh);
        // npc._name = util::FreeTypeUtil::current()->load_npc_str(name, 14);
        // npc._func = util::FreeTypeUtil::current()->load_npc_str(func, 14);

        // npc._name._rect.x = x - npc._name._rect.w / 2;
        // npc._name._rect.y = y + 4;

        // npc._func._rect.x = x - npc._func._rect.w / 2;
        // npc._func._rect.y = y + 20;
        // auto ballon = util::WzUtil::current()->UI->get_root()->find_from_path(u"ChatBalloon.img/0");
        // npc.frame = Frame(ballon);
        // _npc[layer].push_back(npc);
    }
}