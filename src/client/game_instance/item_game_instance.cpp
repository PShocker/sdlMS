#include "src/client/game_instance/item_game_instance.h"

#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system_instance/scene_system_instance.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/common.h"
#include "src/common/request/client_request.h"
#include "src/common/wz/wz_resource.h"
#include "src/server/server_instance/server_character_instance.h"

#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cstdint>
#include <flat_map>
#include <ranges>
#include <string>
namespace sdlms {


bool ItemGameInstance::CheckItem(const std::u16string& id) {
  auto r = id.substr(0, 2);
  if (r == u"01") {
    return false;
  }
  return true;
}

std::u16string ItemGameInstance::LoadItemText(const std::u16string& id,
                                              const std::u16string& val) {
  auto item_type = LoadItemType(id);
  auto str_node = WzResource::string_->find(item_type + u".img");
  if (item_type == u"Etc") {
    str_node = str_node->get_child("Etc");
  }
  auto view = id | std::views::drop_while([](char16_t c) { return c == u'0'; });
  std::u16string result(view.begin(), view.end());
  str_node = str_node->get_child(result)->get_child(val);
  return TextGameInstance::LoadRstr(str_node);
}

std::u16string ItemGameInstance::LoadItemType(const std::u16string& id) {
  auto str = id;
  if (str.length() < 8) {
    str.insert(0, 8 - str.size(), u'0');
  }
  if (str == u"00000000") {
    // meso
    return u"Etc";
  }
  static const std::flat_map<std::u16string, std::u16string> types = {
      {u"05", u"Cash"}, {u"02", u"Consume"}, {u"04", u"Etc"},
      {u"03", u"Ins"},  {u"50", u"Pet"},     {u"09", u"Special"},
  };
  auto r = str.substr(0, 2);
  return types.at(r);
}

wz::Node* ItemGameInstance::LoadItemInfo(const std::u16string& id,
                                         uint32_t num) {
  wz::Node* node;
  if (id == u"00000000") {
    node = WzResource::item_->find(u"Special/0900.img/09000000/iconRaw");
    return node;
  }
  auto type = LoadItemType(id);
  if (type == u"Ins") {
    type = u"Install";
  }
  if (type == u"Consume" || type == u"Etc" || type == u"Install") {
    auto r = id.substr(0, 4) + u".img";
    node = WzResource::item_->find(type + u"/" + r + u"/" + id)
               ->get_child(u"info");
  }
  return node;
}

std::polymorphic<GameItem> ItemGameInstance::LoadItem(const std::u16string& id,
                                                      uint32_t num) {
  auto item_type = ItemGameInstance::LoadItemType(id);
  if (item_type == u"Cash") {
    GameCashItem cash;
    cash.id = id;
    return std::polymorphic<GameItem>(cash);
  } else if (item_type == u"Consume") {
    GameConsumeItem consume;
    consume.id = id;
    consume.num = num;
    return std::polymorphic<GameItem>(consume);
  } else if (item_type == u"Etc") {
    GameEtcItem etc;
    etc.id = id;
    etc.num = num;
    return std::polymorphic<GameItem>(std::in_place_type<GameEtcItem>, etc);
  } else if (item_type == u"Ins") {
    GameInstallItem install;
    install.id = id;
    return std::polymorphic<GameItem>(install);
  } else if (item_type == u"Pet" || item_type == u"Special") {
    GameCashItem cash;
    cash.id = id;
    return std::polymorphic<GameItem>(cash);
  }
  // 处理未知类型 - 返回空或抛出异常
  return std::polymorphic<GameItem>();  // 返回空对象
}

int ItemGameInstance::LoadSlotMax(const std::u16string& id) {
  int r = 200;
  auto info = LoadItemInfo(id, 0);
  if (info->get_child(u"slotMax")) {
    r = static_cast<wz::Property<int>*>(info->get_child(u"slotMax"))->get();
  }
  return r;
}

int ItemGameInstance::LoadItemNum(std::polymorphic<GameItem>& itm) {
  int num = 0;
  switch (itm->type) {
    case ItemEnum::kConsume: {
      auto& consume = static_cast<GameConsumeItem&>(*itm);
      num = consume.num;
      break;
    }
    case ItemEnum::kEtc: {
      auto& etc = static_cast<GameEtcItem&>(*itm);
      num = etc.num;
      break;
    }
    default: {
      break;
    }
  }
  return num;
}

void ItemGameInstance::SetItemNum(std::polymorphic<GameItem>& itm, int num) {
  switch (itm->type) {
    case ItemEnum::kConsume: {
      auto& consume = static_cast<GameConsumeItem&>(*itm);
      consume.num = num;
      break;
    }
    case ItemEnum::kEtc: {
      auto& etc = static_cast<GameEtcItem&>(*itm);
      etc.num = num;
      break;
    }
    default: {
      break;
    }
  }
  QuestGameInstance::UpdateCheckItem();
  return;
}

void ItemGameInstance::AddItemNum(std::polymorphic<GameItem>& item, int num) {
  switch (item->type) {
    case ItemEnum::kConsume: {
      auto& consume = static_cast<GameConsumeItem&>(*item);
      consume.num += num;
      break;
    }
    case ItemEnum::kEtc: {
      auto& etc = static_cast<GameEtcItem&>(*item);
      etc.num += num;
      break;
    }
    default: {
      break;
    }
  }
  QuestGameInstance::UpdateCheckItem();
  return;
}

void ItemGameInstance::DecItemNum(std::polymorphic<GameItem>& item, int num) {
  switch (item->type) {
    case ItemEnum::kEquip: {
      item = std::polymorphic<GameItem>(GameEquipItem{});
      break;
    }
    case ItemEnum::kConsume: {
      auto& consume = static_cast<GameConsumeItem&>(*item);
      if (consume.num >= num) {
        consume.num = consume.num - num;
      } else {
        consume.num = 0;
      }
      if (consume.num == 0) {
        if (!item->id.starts_with(u"0207")) {
          item = std::polymorphic<GameItem>(GameConsumeItem{});
        }
      }
      break;
    }
    case ItemEnum::kEtc: {
      auto& etc = static_cast<GameEtcItem&>(*item);
      etc.num = etc.num - num;
      if (etc.num == 0) {
        item = std::polymorphic<GameItem>(GameEtcItem{});
      }
      break;
    }
    case ItemEnum::kInstall: {
      item = std::polymorphic<GameItem>(GameInstallItem{});
      break;
    }
    case ItemEnum::kCash: {
      item = std::polymorphic<GameItem>(GameCashItem{});
      break;
    }
    default: {
      break;
    }
  }
  QuestGameInstance::UpdateCheckItem();
  return;
}

void ItemGameInstance::UseMorphItem(const std::u16string& id,
                                    GameCharacter& g_character) {
  auto info = ItemGameInstance::LoadItemInfo(id, 0);
  info = info->find(u"../spec");
  auto morph =
      static_cast<wz::Property<int>*>(info->get_child(u"morph"))->get();
  auto tmp = std::format("{:04d}", morph);
  g_character.morph = {tmp.begin(), tmp.end()};

  g_character.action_index = 0;
  g_character.action_time = 0;
  if (&g_character == &CharacterGameInstance::self_) {
    CharacterLogicSystem::RunUnsitChair(g_character);

    fbs::StateT st;
    st.state = fbs::StateEnum_BUFF_ITEM;
    st.val = std::stoi(std::string{id.begin(), id.end()});
    st.sub_val = 1;
    CharacterLogicSystem::ccs_.payload.push_back(
        std::make_unique<fbs::StateT>(st));

    st.state = fbs::StateEnum_ITEM_USE;
    st.sub_val = 1;
    CharacterLogicSystem::ccs_.payload.push_back(
        std::make_unique<fbs::StateT>(st));

    ServerCharacterInstance::HandleMorphUse(g_character, true);
  }
}

void ItemGameInstance::UnuseMorphItem(const std::u16string& id,
                                      GameCharacter& g_character) {
  g_character.morph = u"";
  g_character.action_index = 0;
  g_character.action_time = 0;

  if (&g_character == &CharacterGameInstance::self_) {
    fbs::StateT st;
    st.state = fbs::StateEnum_BUFF_ITEM;
    st.val = std::stoi(std::string{id.begin(), id.end()});
    st.sub_val = 0;

    auto& ccs = CharacterLogicSystem::ccs_;
    ccs.payload.push_back(std::make_unique<fbs::StateT>(st));

    st.state = fbs::StateEnum_ITEM_USE;
    st.sub_val = 0;
    CharacterLogicSystem::ccs_.payload.push_back(
        std::make_unique<fbs::StateT>(st));

    ServerCharacterInstance::HandleMorphUse(g_character, false);
  }
}

void ItemGameInstance::UnuseBuffItem(const std::u16string& id) {
  for (auto& gib : buff_) {
    if (gib.id == id) {
      auto info = ItemGameInstance::LoadItemInfo(id, 0);
      info = info->find(u"../spec");
      if (info->get_child(u"pad")) {
        auto pad =
            static_cast<wz::Property<int>*>(info->get_child(u"pad"))->get();
        auto itm_atk = CharacterStatGameInstance::GetItmAttack();
        CharacterStatGameInstance::SetItmAttack(itm_atk - pad);
      }
      if (info->get_child(u"mad")) {
        auto mad =
            static_cast<wz::Property<int>*>(info->get_child(u"mad"))->get();
        auto itm_magic = CharacterStatGameInstance::GetItmMagic();
        CharacterStatGameInstance::SetItmMagic(itm_magic - mad);
      }
      if (info->get_child(u"acc")) {
        auto acc =
            static_cast<wz::Property<int>*>(info->get_child(u"acc"))->get();
        auto itm_acc = CharacterStatGameInstance::GetItmAcc();
        CharacterStatGameInstance::SetItmAcc(itm_acc - acc);
      }
      if (info->get_child(u"eva")) {
        auto eva =
            static_cast<wz::Property<int>*>(info->get_child(u"eva"))->get();
        auto itm_avd = CharacterStatGameInstance::GetItmAvd();
        CharacterStatGameInstance::SetItmAvd(itm_avd - eva);
      }
      if (info->get_child(u"speed")) {
        auto speed =
            static_cast<wz::Property<int>*>(info->get_child(u"speed"))->get();
        CharacterLogicSystem::self_hspeed_max_ -= 5;
        CharacterLogicSystem::self_hspeed_min_ += 5;
      }
      if (info->get_child(u"crt")) {
        auto crt =
            static_cast<wz::Property<int>*>(info->get_child(u"crt"))->get();
        auto sf_crt = CharacterStatGameInstance::GetCritDamage();
        CharacterStatGameInstance::SetCritDamage(sf_crt - crt);
      }
      if (info->get_child(u"expBuff")) {
        auto expBuff =
            static_cast<wz::Property<int>*>(info->get_child(u"expBuff"))->get();
        auto itm_exp = CharacterStatGameInstance::GetItmExp();
        CharacterStatGameInstance::SetItmExp(itm_exp - expBuff);
      }
      if (info->get_child(u"morph")) {
        auto& sf = CharacterGameInstance::self_;
        UnuseMorphItem(id, sf);
      }
      break;
    }
  }
  std::erase_if(buff_, [id](const auto& buff) { return id == buff.id; });
}

bool ItemGameInstance::UseBuffItem(std::polymorphic<GameItem>& itm) {
  auto id = itm->id;
  auto info = ItemGameInstance::LoadItemInfo(id, 0);
  info = info->find(u"../spec");
  if (info == nullptr) {
    return false;
  }
  if (!info->get_child(u"time")) {
    return false;
  }
  bool r = false;
  UnuseBuffItem(itm->id);
  auto time = static_cast<wz::Property<int>*>(info->get_child(u"time"))->get();
  if (info->get_child(u"pad")) {
    auto pad = static_cast<wz::Property<int>*>(info->get_child(u"pad"))->get();
    auto itm_atk = CharacterStatGameInstance::GetItmAttack();
    CharacterStatGameInstance::SetItmAttack(itm_atk + pad);
    r = true;
  }
  if (info->get_child(u"mad")) {
    auto mad = static_cast<wz::Property<int>*>(info->get_child(u"mad"))->get();
    auto itm_mag = CharacterStatGameInstance::GetItmMagic();
    CharacterStatGameInstance::SetItmMagic(itm_mag + mad);
    r = true;
  }
  if (info->get_child(u"acc")) {
    auto acc = static_cast<wz::Property<int>*>(info->get_child(u"acc"))->get();
    auto itm_acc = CharacterStatGameInstance::GetItmAcc();
    CharacterStatGameInstance::SetItmAcc(acc + itm_acc);
    r = true;
  }
  if (info->get_child(u"eva")) {
    auto eva = static_cast<wz::Property<int>*>(info->get_child(u"eva"))->get();
    auto itm_avd = CharacterStatGameInstance::GetItmAvd();
    CharacterStatGameInstance::SetItmAvd(itm_avd + eva);
    r = true;
  }
  if (info->get_child(u"speed")) {
    CharacterLogicSystem::self_hspeed_max_ += 5;
    CharacterLogicSystem::self_hspeed_min_ -= 5;
    r = true;
  }
  if (info->get_child(u"crt")) {
    auto crt = static_cast<wz::Property<int>*>(info->get_child(u"crt"))->get();
    auto sf_crt = CharacterStatGameInstance::GetCritDamage();
    CharacterStatGameInstance::SetCritDamage(sf_crt + crt);
    r = true;
  }
  if (info->get_child(u"expBuff")) {
    auto expBuff =
        static_cast<wz::Property<int>*>(info->get_child(u"expBuff"))->get();
    auto itm_exp = CharacterStatGameInstance::GetItmExp();
    CharacterStatGameInstance::SetItmExp(itm_exp + expBuff);
  }
  if (info->get_child(u"morph")) {
    UseMorphItem(id, CharacterGameInstance::self_);
    r = true;
  }
  GameItemBuff gib;
  gib.destroy = Window::dt_now_ + time;
  gib.id = id;

  buff_.emplace_back(gib);
  return r;
}

bool ItemGameInstance::UseMobItem(std::polymorphic<GameItem>& itm) {
  bool r = false;
  if (CharacterLogicSystem::self_fh_ == 0) {
    return false;
  }
  auto info = ItemGameInstance::LoadItemInfo(itm->id, 0);
  info = info->find(u"../mob");
  if (info) {
    auto& g_character = CharacterGameInstance::self_;
    for (auto [k, v] : *info->get_children()) {
      auto mob_id =
          static_cast<wz::Property<int>*>(v[0]->get_child(u"id"))->get();
      auto mob_num = 1;
      fbs::ClientCreateMobT ccm;
      ccm.map_id = SceneSystemInstance::map_id_;
      for (int i = 0; i < mob_num; i++) {
        ccm.mobs.emplace_back(std::make_unique<fbs::MobT>());
        auto& mob = ccm.mobs.back();
        mob->mob_id = mob_id;
        mob->state = std::make_unique<fbs::LifeStateT>(fbs::LifeStateT{
            .x = g_character.pos.x,
            .y = g_character.pos.y,
            .action_animate = true,
            .page = g_character.page,
            .fh = static_cast<int16_t>(CharacterLogicSystem::self_fh_),
        });
      }
      ClientRequest::send_to_host(ccm);
      r = true;
    }
  }
  return r;
}

bool ItemGameInstance::UseConsumeItem(const std::u16string& id) {
  bool r = false;
  auto info = ItemGameInstance::LoadItemInfo(id, 0);
  info = info->find(u"../spec");
  if (info == nullptr) {
    return false;
  }
  if (info->get_child(u"hp")) {
    auto hp = static_cast<wz::Property<int>*>(info->get_child(u"hp"))->get();
    auto sf_hp = CharacterStatGameInstance::GetHp();
    CharacterStatGameInstance::SetHp(sf_hp + hp);
    r = true;
  }
  if (info->get_child(u"hpR")) {
    auto hpR = static_cast<wz::Property<int>*>(info->get_child(u"hpR"))->get();
    auto sf_hp_max = CharacterStatGameInstance::GetHpMax();
    auto sf_hp = CharacterStatGameInstance::GetHp();
    CharacterStatGameInstance::SetHp(sf_hp + sf_hp_max * hpR);
    r = true;
  }
  if (info->get_child(u"mp")) {
    auto mp = static_cast<wz::Property<int>*>(info->get_child(u"mp"))->get();
    auto sf_mp = CharacterStatGameInstance::GetMp();
    CharacterStatGameInstance::SetMp(sf_mp + mp);
    r = true;
  }
  if (info->get_child(u"mpR")) {
    auto mpR = static_cast<wz::Property<int>*>(info->get_child(u"mpR"))->get();
    auto sf_mp_max = CharacterStatGameInstance::GetMpMax();
    auto sf_mp = CharacterStatGameInstance::GetMp();
    CharacterStatGameInstance::SetMp(sf_mp + sf_mp_max * mpR);
    r = true;
  }
  if (info->get_child(u"moveTo")) {
    auto moveTo =
        static_cast<wz::Property<int>*>(info->get_child(u"moveTo"))->get();
    SceneSystemInstance::EnterPrepare(moveTo, u"sp", 0);
    r = true;
  }
  if (info->get_child(u"exp")) {
    auto exp = static_cast<wz::Property<int>*>(info->get_child(u"exp"))->get();
    CharacterStatGameInstance::AddExp(exp);
    r = true;
  }
  return r;
}

bool ItemGameInstance::UseConsumeItem(std::polymorphic<GameItem>& itm) {
  bool r = false;
  if (UseBuffItem(itm)) {
    r = true;
  }
  if (UseConsumeItem(itm->id)) {
    r = true;
  }
  if (UseMobItem(itm)) {
    r = true;
  }
  return r;
}

void ItemGameInstance::UseItem(std::polymorphic<GameItem>& itm) {
  auto item_type = ItemGameInstance::LoadItemType(itm->id);
  auto& g_character = CharacterGameInstance::self_;
  auto self_sit_cooldown = CharacterLogicSystem::self_sit_cooldown_;

  if (item_type == u"Ins") {
    if (itm->id.starts_with(u"0301") && self_sit_cooldown < Window::dt_now_) {
      if (!g_character.morph.empty()) {
        return;
      }
      auto action_type = CharacterLogicSystem::LoadActionType(g_character);
      switch (action_type) {
        case CharacterLogicSystem::ActionEnum::kStand: {
          CharacterLogicSystem::RunSitChair(g_character, itm->id);
          break;
        }
        case CharacterLogicSystem::ActionEnum::kSit: {
          CharacterLogicSystem::RunUnsitChair(g_character);
          break;
        }
        default: {
          break;
        }
      }
      return;
    }
  } else if (item_type == u"Consume") {
    if (UseConsumeItem(itm)) {
      DecItemNum(itm, 1);
    }
  }
  return;
}

}  // namespace sdlms
