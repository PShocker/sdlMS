#include "game_save_system_instance.h"
#include "SDL3/SDL_filesystem.h"
#include "scene_system_instance.h"
#include "src/client/game/game_equip.h"
#include "src/client/game/game_item.h"
#include "src/client/game/game_save.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/system/ui/character_choose_ui_system.h"
#include "src/common/flatbuffers/common.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
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
      cs.character = character_game_instance::load_g_character(c->character);
      auto name =
          std::u16string{c->character->name.begin(), c->character->name.end()};
      character_game_instance::load_name(cs.character, name);
      for (auto &item : c->package) {
        switch (item->data.type) {
        case fbs::ItemUnion_Equip: {
          auto eqp = item->data.AsEquip();
          game_equip g_equip;
          g_equip.id = eqp->equip_id;
          cs.package.push_back({
              .index = item->index,
              .val = g_equip,
          });
          break;
        }
        case fbs::ItemUnion_Item: {
          auto itm = item->data.AsItem();
          game_item g_item;
          g_item.id = itm->item_id;
          g_item.num = itm->item_num;
          cs.package.push_back({
              .index = item->index,
              .val = g_item,
          });
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

      for (auto &i : c->sp) {
        cs.sp.ski_sp[i->id] = i->val;
      }
      save.characters.push_back(cs);
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
  if (!character.nametags.empty()) {
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
    cs.meso = package_game_instance::meso;

    cs.hp = character_stat_game_instance::hp_point;
    cs.mp = character_stat_game_instance::mp_point;
    cs.exp = character_stat_game_instance::exp_point;

    for (uint32_t i = 0; i < package_game_instance::equips.size(); i++) {
      auto equip = package_game_instance::equips[i];
      if (equip.has_value()) {
        cs.package.push_back({
            .index = i,
            .val = equip.value(),
        });
      }
    }
    for (uint32_t i = 0; i < package_game_instance::equips.size(); i++) {
      auto equip = package_game_instance::equips[i];
      if (equip.has_value()) {
        auto &eqp = equip.value();
        cs.package.push_back({
            .index = i,
            .val = eqp,
        });
      }
    }
    for (auto &v : {
             package_game_instance::cosumes,
             package_game_instance::etc,
             package_game_instance::install,
             package_game_instance::cash,
         }) {
      for (uint32_t i = 0; i < v.size(); i++) {
        auto item = v[i];
        if (item.has_value()) {
          auto &itm = item.value();
          cs.package.push_back({
              .index = i,
              .val = itm,
          });
        }
      }
    }
    for (int i = 0; i < save.characters.size(); i++) {
      auto &save_character = save.characters[i].character;
      if (save_character.nametags[0].text == character.nametags[0].text) {
        save.characters[i] = cs;
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
    auto character_s = save.characters[i];
    auto ct = character_game_instance::load_characterT(character_s.character);
    cst.character = std::make_unique<CharacterT>(ct);
    cst.ap = std::make_unique<APSaveT>();
    cst.ap->hp_ap = character_s.ap.hp_ap;
    cst.ap->mp_ap = character_s.ap.mp_ap;

    cst.ap->str_ap = character_s.ap.str_ap;
    cst.ap->dex_ap = character_s.ap.dex_ap;
    cst.ap->int_ap = character_s.ap.int_ap;
    cst.ap->luk_ap = character_s.ap.luk_ap;

    cst.map_id = character_s.map_id;
    cst.meso = character_s.meso;

    cst.hp = character_s.hp;
    cst.mp = character_s.mp;
    cst.exp = character_s.exp;

    for (auto [k, v] : character_s.sp.ski_sp) {
      SPSaveT spt = {
          .id = k,
          .val = v,
      };
      cst.sp.push_back(std::make_unique<SPSaveT>(spt));
    }
    for (auto &pkg : character_s.package) {
      PackageSaveT pst;
      pst.index = pkg.index;
      if (std::holds_alternative<game_equip>(pkg.val)) {
        game_equip &equip = std::get<game_equip>(pkg.val);
        EquipT et;
        et.equip_id = std::stoi(std::string{equip.id.begin(), equip.id.end()});
        pst.data.Set(et);
      } else {
        game_item &item = std::get<game_item>(pkg.val);
        ItemT it;
        it.item_id = std::stoi(std::string{item.id.begin(), item.id.end()});
        it.item_num = item.num;
        pst.data.Set(it);
      }
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