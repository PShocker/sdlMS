#include "game_save_system_instance.h"
#include "SDL3/SDL_filesystem.h"
#include "src/client/game/game_save.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/common/flatbuffers/common.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>

bool game_save_system_instance::load_save(const std::string &login) {
  if (!SDL_CreateDirectory("./Save")) {
    assert(0);
    std::abort();
  }
  size_t file_size = 0;
  std::string path = "./Save/" + login + ".bin";
  void *data = SDL_LoadFile(path.c_str(), &file_size);
  if (data != nullptr) {

    SDL_free(data);
    return true;
  }
  save = {};
  save.username = login;
  return false;
}

bool game_save_system_instance::save_game() {
  // save
  auto character = character_game_instance::self;
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
  cs.meso = package_game_instance::meso;
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
  if (!save.characters.empty()) {
    for (int i = 0; i < save.characters.size(); i++) {
      auto &save_character = save.characters[i].character;
      if (save_character.nametags[0].text == character.nametags[0].text) {
        save.characters[i] = cs;
        break;
      }
    }
  } else {
    save.characters.push_back(cs);
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

    for (auto [k, v] : character_s.sp.ski_sp) {
      SPSaveT spt = {
          .id = k,
          .val = v,
      };
      cst.sp.push_back(std::make_unique<SPSaveT>(spt));
    }
    for (auto &pkg : character_s.package) {
      PackageSaveT pst;
      pst.id = pkg.index;
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
    gst.data = std::make_unique<PlayerSaveT>(pst);
    flatbuffers::FlatBufferBuilder builder;
    auto offset = GameSave::Pack(builder, &gst);
    builder.Finish(offset);
    uint8_t *pointer = builder.GetBufferPointer();
    size_t size = builder.GetSize();
    
  }

  return true;
}