#include "game_save_system_instance.h"
#include "SDL3/SDL_filesystem.h"
#include "scene_system_instance.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_quest.h"
#include "src/client/game/game_save.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/keyboard_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/portal_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/common/flatbuffers/common.h"
#include "src/server/server_instance/server_character_instance.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <flat_map>
#include <format>
#include <memory>
#include <string>

bool game_save_system_instance::load_save(const std::string &login) {
  save = {};
  save.username = login;
  if (!SDL_CreateDirectory("./Save")) {
    assert(0);
    std::abort();
  }
  size_t file_size = 0;
  std::string path = "./Save/" + login + ".bin";
  void *data = SDL_LoadFile(path.c_str(), &file_size);
  if (data != nullptr) {
    auto game_save = flatbuffers::GetRoot<GameSave>(data);
    GameSaveT gst;
    game_save->UnPackTo(&gst);
    for (const auto &c : gst.data->data) {
      character_save cs;
      cs.map_id = c->map_id;
      cs.character = server_character_instance::load_g_character(c->character);
      std::u16string name{c->character->name.begin(), c->character->name.end()};
      cs.character.name = name;
      for (auto &item : c->package) {
        switch (item->data.type) {
        case fbs::ItemUnion_Equip: {
          auto eqp = item->data.AsEquip();
          game_equip_item g_equip;
          auto tmp = std::format("{:08d}", eqp->equip_id);
          g_equip.id = {tmp.begin(), tmp.end()};
          for (const auto &scroll : eqp->scroll) {
            tmp = std::format("{:08d}", scroll->scroll_id);
            std::u16string scroll_id{tmp.begin(), tmp.end()};
            g_equip.scroll.push_back({scroll_id, scroll->success});
          }
          auto g_item = std::polymorphic<game_item>(
              std::in_place_type<game_equip_item>, g_equip);
          cs.package.emplace_back(item->index, g_item);
          break;
        }
        case fbs::ItemUnion_Item: {
          auto itm = item->data.AsItem();
          auto tmp = std::format("{:08d}", itm->item_id);
          std::u16string item_id{tmp.begin(), tmp.end()};
          auto item2 = item_game_instance::load_item(item_id, itm->item_num);
          cs.package.emplace_back(item->index, std::move(item2));
          break;
        }
        default: {
          break;
        }
        }
      }
      cs.meso = c->meso;
      cs.ap.hp_ap = c->ap->hp_ap;
      cs.ap.mp_ap = c->ap->mp_ap;

      cs.ap.str_ap = c->ap->str_ap;
      cs.ap.dex_ap = c->ap->dex_ap;
      cs.ap.int_ap = c->ap->int_ap;
      cs.ap.luk_ap = c->ap->luk_ap;

      cs.exp = c->exp;

      for (const auto &quest : c->quest) {
        game_quest g_quest;
        g_quest.quest_id = {quest->id.begin(), quest->id.end()};
        g_quest.index = quest->index;
        g_quest.type = (quest_enum)quest->type;
        for (const auto &v : quest->mob) {
          quest_mob q_mob;
          auto tmp = std::format("{:07d}", v->mob_id);
          q_mob.id = {tmp.begin(), tmp.end()};
          q_mob.count = v->mob_num;
          g_quest.mob.push_back(q_mob);
        }
        for (const auto &v : quest->npc) {
          quest_npc q_npc;
          auto tmp = std::format("{:07d}", v->npc_id);
          q_npc.id = {tmp.begin(), tmp.end()};
          g_quest.npc.push_back(q_npc);
        }
        cs.quests.push_back(g_quest);
      }

      for (const auto &k : c->key) {
        key_save ks;
        ks.scan_code = k->scan_code;
        ks.type = k->type;
        ks.val = k->val;
        ks.sub_val = k->sub_val;
        cs.keys.push_back(ks);
      }

      for (auto &i : c->sp) {
        cs.sp.ski_sp[i->id] = i->val;
      }
      save.characters.push_back(std::move(cs));
    }
    for (const auto &s : gst.data->storage) {
      switch (s.type) {
      case fbs::ItemUnion_Equip: {
        auto eqp = s.AsEquip();
        game_equip_item g_equip;
        auto tmp = std::format("{:08d}", eqp->equip_id);
        g_equip.id = {tmp.begin(), tmp.end()};
        auto g_item = std::polymorphic<game_item>(
            std::in_place_type<game_equip_item>, g_equip);
        break;
      }
      case fbs::ItemUnion_Item: {
        break;
      }
      default: {
        break;
      }
      }
    }
    SDL_free(data);
    return true;
  }
  return false;
}

bool game_save_system_instance::save_game() {
  if (save.username.empty()) {
    return false;
  }
  if (character_choose_ui_system::characters.empty()) {
    return false;
  }
  // save
  auto character = character_game_instance::self;
  if (!character.name.empty()) {
    character_save cs;
    cs.character = character;
    cs.ap = {
        .hp_ap = character_stat_game_instance::hp_ap,
        .mp_ap = character_stat_game_instance::mp_ap,
        .str_ap = character_stat_game_instance::str_ap,
        .dex_ap = character_stat_game_instance::dex_ap,
        .int_ap = character_stat_game_instance::int_ap,
        .luk_ap = character_stat_game_instance::luk_ap,
    };
    cs.sp = {job_skill_game_instance::skill_point};

    cs.map_id = scene_system_instance::map_id;

    auto [first, last] = portal_game_instance::data.equal_range(u"sp");
    std::flat_map<uint64_t, int> por_dis;
    const auto &character_pos = character.pos;
    for (auto it = first; it != last; ++it) {
      const auto &portal = it->second;
      auto dx = portal.pos.x - character_pos.x;
      auto dy = portal.pos.y - character_pos.y;
      por_dis[dx * dx + dy * dy] = static_cast<int>(std::distance(first, it));
    }
    cs.portal_id = por_dis.begin()->second;

    cs.meso = package_game_instance::meso;

    cs.hp = character_stat_game_instance::hp_point;
    cs.mp = character_stat_game_instance::mp_point;
    cs.exp = character_stat_game_instance::exp_point;

    for (auto [k, v] : quest_game_instance::progress_quests) {
      cs.quests.push_back(std::move(v));
    }
    for (auto [k, v] : quest_game_instance::complete_quests) {
      cs.quests.push_back(std::move(v));
    }
    for (auto [k, v] : quest_game_instance::decline_quests) {
      cs.quests.push_back(std::move(v));
    }

    for (auto [k, v] : keyboard_game_instance::data) {
      cs.keys.push_back({
          .scan_code = static_cast<uint16_t>(k),
          .type = v.type,
          .val = v.val,
          .sub_val = v.sub_val,
      });
    }

    for (auto &d : package_game_instance::data) {
      for (uint32_t i = 0; i < d.size(); ++i) {
        auto &v = d[i];
        if (!v->id.empty()) {
          cs.package.push_back({.index = i, .val = std::move(v)});
        }
      }
    }
    for (int i = 0; i < save.characters.size(); i++) {
      auto &save_character = save.characters[i].character;
      if (save_character.name == character.name) {
        save.characters[i] = std::move(cs);
        break;
      }
    }
  }

  // save
  GameSaveT gst;
  PlayerSaveT pst;
  gst.username = save.username;
  for (uint32_t i = 0; i < save.characters.size(); i++) {
    CharacterSaveT cst;
    auto character_s = std::move(save.characters[i]);
    auto ct = server_character_instance::load_charactert(character_s.character);
    cst.character = std::make_unique<CharacterT>(ct);
    cst.ap = std::make_unique<APSaveT>();
    cst.ap->hp_ap = character_s.ap.hp_ap;
    cst.ap->mp_ap = character_s.ap.mp_ap;

    cst.ap->str_ap = character_s.ap.str_ap;
    cst.ap->dex_ap = character_s.ap.dex_ap;
    cst.ap->int_ap = character_s.ap.int_ap;
    cst.ap->luk_ap = character_s.ap.luk_ap;

    cst.map_id = character_s.map_id;
    cst.portal_id = character_s.portal_id;
    cst.meso = character_s.meso;

    cst.hp = character_s.hp;
    cst.mp = character_s.mp;
    cst.exp = character_s.exp;

    for (auto &quest : character_s.quests) {
      QuestSaveT qt;
      qt.id = {quest.quest_id.begin(), quest.quest_id.end()};
      qt.index = quest.index;
      qt.type = (QuestEnum)quest.type;
      for (const auto &v : quest.mob) {
        QuestMobSaveT qmt;
        qmt.mob_id = std::stoi(std::string{v.id.begin(), v.id.end()});
        qmt.mob_num = v.count;
        qt.mob.push_back(std::make_unique<QuestMobSaveT>(qmt));
      }
      for (const auto &v : quest.npc) {
        QuestNPCSaveT qnt;
        qnt.npc_id = std::stoi(std::string{v.id.begin(), v.id.end()});
        qt.npc.push_back(std::make_unique<QuestNPCSaveT>(qnt));
      }
      cst.quest.push_back(std::make_unique<QuestSaveT>(qt));
    }

    for (auto &k : character_s.keys) {
      KeyConfigSaveT kcs;
      kcs.scan_code = k.scan_code;
      kcs.type = k.type;
      kcs.val = k.val;
      kcs.sub_val = k.sub_val;
      cst.key.push_back(std::make_unique<KeyConfigSaveT>(kcs));
    }

    for (auto [k, v] : character_s.sp.ski_sp) {
      SPSaveT spt = {
          .id = k,
          .val = v,
      };
      cst.sp.push_back(std::make_unique<SPSaveT>(spt));
    }
    for (auto &pkg : character_s.package) {
      PackageSaveT pst;
      uint32_t item_num = 1;
      uint32_t item_id =
          std::stoi(std::string{pkg.val->id.begin(), pkg.val->id.end()});
      switch (pkg.val->type) {
      case item_enum::equip: {
        game_equip_item &equip = static_cast<game_equip_item &>(*pkg.val);
        EquipT et;
        et.equip_id = item_id;
        for (auto &scroll : equip.scroll) {
          EquipScrollT est;
          est.scroll_id =
              std::stoi(std::string{scroll.id.begin(), scroll.id.end()});
          est.success = scroll.success;
          et.scroll.push_back(std::make_unique<EquipScrollT>(est));
        }
        pst.data.Set(et);
        break;
      }
      case item_enum::consume: {
        game_consume_item &consume = static_cast<game_consume_item &>(*pkg.val);
        item_num = consume.num;
      }
      case item_enum::etc: {
        game_etc_item &etc = static_cast<game_etc_item &>(*pkg.val);
        item_num = etc.num;
      }
      case item_enum::install: {
      }
      case item_enum::cash: {
        ItemT it;
        it.item_id = item_id;
        it.item_num = item_num;
        pst.data.Set(it);
        break;
      }
      case item_enum::deco: {
        EquipT et;
        et.equip_id = item_id;
        pst.data.Set(et);
        break;
      }
      }
      pst.expire = pkg.val->expire;
      pst.index = pkg.index;
      cst.package.push_back(std::make_unique<PackageSaveT>(pst));
    }
    pst.data.push_back(std::make_unique<CharacterSaveT>(cst));
  }
  gst.data = std::make_unique<PlayerSaveT>(pst);
  flatbuffers::FlatBufferBuilder builder;
  auto offset = GameSave::Pack(builder, &gst);
  builder.Finish(offset);
  uint8_t *pointer = builder.GetBufferPointer();
  size_t size = builder.GetSize();
  auto save_path = "./Save/" + save.username + ".bin";
  SDL_IOStream *io = SDL_IOFromFile(save_path.c_str(), "wb");
  if (io == NULL) {
    return -1;
  }
  SDL_WriteIO(io, pointer, size);
  SDL_CloseIO(io);
  return true;
}