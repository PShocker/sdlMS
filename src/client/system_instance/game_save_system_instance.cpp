#include "src/client/system_instance/game_save_system_instance.h"

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
#include "src/client/system_instance/scene_system_instance.h"
#include "src/common/flatbuffers/common.h"
#include "src/server/server_instance/server_character_instance.h"

#include <SDL3/SDL_filesystem.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <flat_map>
#include <format>
#include <memory>
#include <string>

namespace sdlms {

bool GameSaveSystemInstance::LoadSave(const std::string& login) {
  save_ = {};
  save_.username = login;
  if (!SDL_CreateDirectory("./Save")) {
    assert(0);
    std::abort();
  }
  size_t file_size = 0;
  std::string path = "./Save/" + login + ".bin";
  void* data = SDL_LoadFile(path.c_str(), &file_size);
  if (data != nullptr) {
    auto root = flatbuffers::GetRoot<fbs::GameSave>(data);
    fbs::GameSaveT gst;
    root->UnPackTo(&gst);
    for (const auto& c : gst.data->data) {
      CharacterSaveData cs;
      cs.map_id = c->map_id;
      cs.character = ServerCharacterInstance::LoadGCharacter(c->character);
      std::u16string name{c->character->name.begin(), c->character->name.end()};
      cs.character.name = name;
      for (auto& item : c->package) {
        switch (item->data.type) {
          case fbs::ItemUnion_Equip: {
            auto eqp = item->data.AsEquip();
            GameEquipItem g_equip;
            auto tmp = std::format("{:08d}", eqp->equip_id);
            g_equip.id = {tmp.begin(), tmp.end()};
            for (const auto& scroll : eqp->scroll) {
              tmp = std::format("{:08d}", scroll->scroll_id);
              std::u16string scroll_id{tmp.begin(), tmp.end()};
              g_equip.scroll.push_back({scroll_id, scroll->success});
            }
            auto g_item = std::polymorphic<GameItem>(
                std::in_place_type<GameEquipItem>, g_equip);
            cs.package.emplace_back(item->index, g_item);
            break;
          }
          case fbs::ItemUnion_Item: {
            auto itm = item->data.AsItem();
            auto tmp = std::format("{:08d}", itm->item_id);
            std::u16string item_id{tmp.begin(), tmp.end()};
            auto item2 = ItemGameInstance::LoadItem(item_id, itm->item_num);
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

      for (const auto& quest : c->quest) {
        GameQuest g_quest;
        g_quest.quest_id = {quest->id.begin(), quest->id.end()};
        g_quest.index = quest->index;
        g_quest.type = (QuestEnum)quest->type;
        for (const auto& v : quest->mob) {
          QuestMob q_mob;
          auto tmp = std::format("{:07d}", v->mob_id);
          q_mob.id = {tmp.begin(), tmp.end()};
          q_mob.count = v->mob_num;
          g_quest.mob[q_mob.id] = q_mob;
        }
        cs.quests.push_back(g_quest);
      }

      for (const auto& k : c->key) {
        KeySave ks;
        ks.scan_code = k->scan_code;
        ks.type = k->type;
        ks.val = k->val;
        ks.sub_val = k->sub_val;
        cs.keys.push_back(ks);
      }

      for (auto& i : c->sp) {
        cs.sp.ski_sp[i->id] = i->val;
      }
      save_.characters.push_back(std::move(cs));
    }
    for (const auto& s : gst.data->storage) {
      switch (s.type) {
        case fbs::ItemUnion_Equip: {
          auto eqp = s.AsEquip();
          GameEquipItem g_equip;
          auto tmp = std::format("{:08d}", eqp->equip_id);
          g_equip.id = {tmp.begin(), tmp.end()};
          auto g_item = std::polymorphic<GameItem>(
              std::in_place_type<GameEquipItem>, g_equip);
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

bool GameSaveSystemInstance::SaveGame() {
  if (save_.username.empty()) {
    return false;
  }
  if (CharacterChooseUiSystem::characters_.empty()) {
    return false;
  }
  // save
  auto character = CharacterGameInstance::self_;
  if (!character.name.empty()) {
    CharacterSaveData cs;
    cs.character = character;
    cs.ap.hp_ap = CharacterStatGameInstance::GetHpAp(),
    cs.ap.mp_ap = CharacterStatGameInstance::GetMpAp(),
    cs.ap.str_ap = CharacterStatGameInstance::GetStrAp(),
    cs.ap.dex_ap = CharacterStatGameInstance::GetDexAp(),
    cs.ap.int_ap = CharacterStatGameInstance::GetIntAp(),
    cs.ap.luk_ap = CharacterStatGameInstance::GetLukAp(),
    cs.sp = {JobSkillGameInstance::skill_point_};

    cs.map_id = SceneSystemInstance::map_id_;

    auto [first, last] = PortalGameInstance::data_.equal_range(u"sp");
    std::flat_map<uint64_t, int> por_dis;
    const auto& character_pos = character.pos;
    for (auto it = first; it != last; ++it) {
      const auto& portal = it->second;
      auto dx = portal.pos.x - character_pos.x;
      auto dy = portal.pos.y - character_pos.y;
      por_dis[dx * dx + dy * dy] = static_cast<int>(std::distance(first, it));
    }
    cs.portal_id = por_dis.begin()->second;

    cs.meso = PackageGameInstance::meso_;

    cs.hp = CharacterStatGameInstance::GetHp();
    cs.mp = CharacterStatGameInstance::GetMp();
    cs.exp = CharacterStatGameInstance::GetExp();

    for (auto [k, v] : QuestGameInstance::progress_quests_) {
      cs.quests.push_back(std::move(v));
    }
    for (auto [k, v] : QuestGameInstance::complete_quests_) {
      cs.quests.push_back(std::move(v));
    }
    for (auto [k, v] : QuestGameInstance::decline_quests_) {
      cs.quests.push_back(std::move(v));
    }

    for (auto [k, v] : KeyboardGameInstance::data_) {
      cs.keys.push_back({
          .scan_code = static_cast<uint16_t>(k),
          .type = v.type,
          .val = v.val,
          .sub_val = v.sub_val,
      });
    }

    for (auto& d : PackageGameInstance::data_) {
      for (uint32_t i = 0; i < d.size(); ++i) {
        auto& v = d[i];
        if (!v->id.empty()) {
          cs.package.push_back({.index = i, .val = std::move(v)});
        }
      }
    }
    for (int i = 0; i < save_.characters.size(); i++) {
      auto& save_character = save_.characters[i].character;
      if (save_character.name == character.name) {
        save_.characters[i] = std::move(cs);
        break;
      }
    }
  }

  // save
  fbs::GameSaveT gst;
  fbs::PlayerSaveT pst;
  gst.username = save_.username;
  for (uint32_t i = 0; i < save_.characters.size(); i++) {
    fbs::CharacterSaveT cst;
    auto character_s = std::move(save_.characters[i]);
    auto ct = ServerCharacterInstance::LoadCharactert(character_s.character);
    cst.character = std::make_unique<fbs::CharacterT>(ct);
    cst.ap = std::make_unique<fbs::APSaveT>();
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

    for (auto& quest : character_s.quests) {
      fbs::QuestSaveT qt;
      qt.id = {quest.quest_id.begin(), quest.quest_id.end()};
      qt.index = quest.index;
      qt.type = (uint8_t)quest.type;
      for (const auto& v : quest.mob | std::views::values) {
        fbs::QuestMobSaveT qmt;
        qmt.mob_id = std::stoi(std::string{v.id.begin(), v.id.end()});
        qmt.mob_num = v.count;
        qt.mob.push_back(std::make_unique<fbs::QuestMobSaveT>(qmt));
      }
      cst.quest.push_back(std::make_unique<fbs::QuestSaveT>(qt));
    }

    for (auto& k : character_s.keys) {
      fbs::KeyConfigSaveT kcs;
      kcs.scan_code = k.scan_code;
      kcs.type = k.type;
      kcs.val = k.val;
      kcs.sub_val = k.sub_val;
      cst.key.push_back(std::make_unique<fbs::KeyConfigSaveT>(kcs));
    }

    for (auto [k, v] : character_s.sp.ski_sp) {
      fbs::SPSaveT spt = {
          .id = k,
          .val = v,
      };
      cst.sp.push_back(std::make_unique<fbs::SPSaveT>(spt));
    }
    for (auto& pkg : character_s.package) {
      fbs::PackageSaveT pst;
      uint32_t item_num = 1;
      uint32_t item_id =
          std::stoi(std::string{pkg.val->id.begin(), pkg.val->id.end()});
      switch (pkg.val->type) {
        case ItemEnum::kEquip: {
          GameEquipItem& equip = static_cast<GameEquipItem&>(*pkg.val);
          fbs::EquipT et;
          et.equip_id = item_id;
          for (auto& scroll : equip.scroll) {
            fbs::EquipScrollT est;
            est.scroll_id =
                std::stoi(std::string{scroll.id.begin(), scroll.id.end()});
            est.success = scroll.success;
            et.scroll.push_back(std::make_unique<fbs::EquipScrollT>(est));
          }
          pst.data.Set(et);
          break;
        }
        case ItemEnum::kConsume: {
          GameConsumeItem& consume = static_cast<GameConsumeItem&>(*pkg.val);
          item_num = consume.num;
          [[fallthrough]];
        }
        case ItemEnum::kEtc: {
          GameEtcItem& etc = static_cast<GameEtcItem&>(*pkg.val);
          item_num = etc.num;
          [[fallthrough]];
        }
        case ItemEnum::kInstall: {
        }
        case ItemEnum::kCash: {
          fbs::ItemT it;
          it.item_id = item_id;
          it.item_num = item_num;
          pst.data.Set(it);
          break;
        }
        case ItemEnum::kDeco: {
          fbs::EquipT et;
          et.equip_id = item_id;
          pst.data.Set(et);
          break;
        }
      }
      pst.expire = pkg.val->expire;
      pst.index = pkg.index;
      cst.package.push_back(std::make_unique<fbs::PackageSaveT>(pst));
    }
    pst.data.push_back(std::make_unique<fbs::CharacterSaveT>(cst));
  }
  gst.data = std::make_unique<fbs::PlayerSaveT>(pst);
  flatbuffers::FlatBufferBuilder builder;
  auto offset = fbs::GameSave::Pack(builder, &gst);
  builder.Finish(offset);
  uint8_t* pointer = builder.GetBufferPointer();
  size_t size = builder.GetSize();
  auto save_path = "./Save/" + save_.username + ".bin";
  SDL_IOStream* io = SDL_IOFromFile(save_path.c_str(), "wb");
  if (io == NULL) {
    return -1;
  }
  SDL_WriteIO(io, pointer, size);
  SDL_CloseIO(io);
  return true;
}

}  // namespace sdlms
