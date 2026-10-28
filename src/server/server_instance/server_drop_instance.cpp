#include "server_drop_instance.h"
#include "server_scene_instance.h"
#include "src/client/game/game_drop.h"
#include "src/client/game/game_gain_log.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/drop_game_instance.h"
#include "src/client/game_instance/gain_log_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/map_info_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/window/window.h"
#include "src/common/flatbuffers/server.h"
#include "src/common/physic/physic.h"
#include "src/common/response/server_response.h"
#include "src/server/server/server_drop.h"
#include "src/server/server_main.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <optional>
#include <utility>
namespace sdlms {


float ServerDropInstance::CalAvailableTime(float y1, float y2) {
  const float VY = 400.0f;                     // 初速度（向上）
  const float G = 800.0f;                      // 重力加速度
  const float MAX_RISE = VY * VY / (2.0f * G); // 最大上升高度 = 100.0f

  float max_height = y1 - MAX_RISE;

  // 总下落距离 = 从最高点到y2
  float total_fall_distance = y2 - max_height;

  if (total_fall_distance < 0) {
    // y2在最高点上方，无法到达
    return -1.0f;
  }

  // 上升到最高点的时间
  float rise_time = VY / G; // 0.5s

  // 从最高点下落到y2的时间
  float fall_time = sqrtf(2.0f * total_fall_distance / G);

  return rise_time + fall_time;
}

void ServerDropInstance::SaveDrop(uint64_t map_id, const fbs::DropT &drop) {
  auto &scene = ServerSceneInstance::scenes_[map_id];
  ServerDrop sd;
  sd.dt = drop;
  sd.destroy = Window::dt_now_ + 60 * 5 * 1000;
  sd.available = Window::dt_now_ + CalAvailableTime(drop.y1, drop.y2) * 1000;
  scene.drops.emplace(drop.random_id, sd);
}

void ServerDropInstance::SaveDelayDrop(uint64_t map_id, uint64_t d,
                                           const fbs::DropT &drop) {
  auto &scene = ServerSceneInstance::scenes_[map_id];
  ServerDrop sd;
  sd.dt = drop;
  sd.destroy = Window::dt_now_ + (d - Window::dt_time_) + 60 * 5 * 1000;
  sd.available = Window::dt_now_ + (d - Window::dt_time_) +
                 CalAvailableTime(drop.y1, drop.y2) * 1000;
  scene.delay_drops[d].emplace_back(sd);
}

void ServerDropInstance::HandleClientDrop(uint64_t client_id,
                                              fbs::ClientCharacterDropT &r) {
  auto map_id = r.map_id;

  auto dts = CreateDts({*r.payload}, map_id);
  for (const auto &dt : dts) {
    SaveDrop(map_id, *dt);
  }
  const auto &clients = ServerSceneInstance::scenes_[map_id].clients;
  fbs::ServerCharacterDropT t;
  t.client_id = client_id;
  t.payload = std::move(dts[0]);
  for (const auto c : clients) {
    ServerResponse::send_to_client(c, t);
  }
}

void ServerDropInstance::HandlePick(uint64_t client_id,
                                       fbs::ClientCharacterPickT &r) {
  auto map_id = r.map_id;
  auto &scene = ServerSceneInstance::scenes_[map_id];
  if (scene.drops.contains(r.random_id)) {
    if (scene.drops[r.random_id].available <= Window::dt_now_) {
      fbs::ServerCharacterPickT t;
      t.random_id = r.random_id;
      t.client_id = client_id;
      auto clients = ServerSceneInstance::scenes_[map_id].clients;
      for (const auto c : clients) {
        ServerResponse::send_to_client(c, t);
      }
      scene.drops.erase(r.random_id);
    } else {
      fbs::ServerCharacterPickT t;
      t.random_id = 0;
      ServerResponse::send_to_client(client_id, t);
    }
  }
}

void ServerDropInstance::HandleServerPick(uint64_t client_id,
                                              fbs::ServerCharacterPickT &r) {
  if (r.client_id == ServerMain::local_addr_ && r.random_id == 0) {
    // 捡取物品失败
    CharacterLogicSystem::ccp_ = {};
    return;
  }
  if (DropGameInstance::data_.contains(r.random_id)) {
    auto &dt = DropGameInstance::data_.at(r.random_id);
    GameDropPick gdp;
    gdp.client_id = r.client_id;
    if (r.pet) {
      gdp.pet_index = r.pet_index;
    }
    dt.picker = gdp;
    dt.pick_time = Window::dt_now_;
    dt.type = GameDrop::DropEnum::kPick;

    if (r.client_id == ServerMain::local_addr_) {
      auto itm = dt.data;
      PackageGameInstance::AddNewItem(itm);
      AudioGameInstance::LoadAudio(u"Game.img/PickUpItem", 0);

      CharacterLogicSystem::ccp_ = {};

      auto itm_num = ItemGameInstance::LoadItemNum(dt.data);
      itm_num = std::max(1, itm_num);
      GainLogGameInstance::Add(dt.data->id, itm_num, GainEnum::kItem);
    }
  }
}

void ServerDropInstance::HandleServerDt(const fbs::DropT &dt) {
  GameDrop drop;
  drop.random_id = dt.random_id;
  drop.page = dt.page;
  drop.vspeed = -400;
  drop.pos = SDL_FPoint{dt.x1, dt.y1};
  drop.goal = SDL_FPoint{dt.x2, dt.y2};
  drop.hspeed = (dt.x2 - dt.x1) / (-drop.vspeed / 800);

  switch (dt.drop.type) {
  case fbs::ItemUnion_Equip: {
    auto equipT = dt.drop.AsEquip();
    GameEquipItem equip;
    auto tmp = std::format("{:08d}", equipT->equip_id);
    equip.id = {tmp.begin(), tmp.end()};
    drop.data =
        std::polymorphic<GameItem>(std::in_place_type<GameEquipItem>, equip);
    break;
  }
  case fbs::ItemUnion_Item: {
    auto item = dt.drop.AsItem();
    auto tmp = std::format("{:08d}", item->item_id);
    drop.data =
        ItemGameInstance::LoadItem({tmp.begin(), tmp.end()}, item->item_num);
    break;
  }
  default: {
    break;
  }
  }
  DropGameInstance::data_.emplace(dt.random_id, drop);
}

void ServerDropInstance::HandleServerSceneDt(const fbs::DropT &dt) {
  HandleServerDt(dt);
  auto &drop = DropGameInstance::data_.at(dt.random_id);
  drop.hspeed = 0;
  drop.pos.x = dt.x2;
  drop.pos.y = dt.y2;
  drop.type = GameDrop::DropEnum::kLand;
  return;
}

void ServerDropInstance::HandleServerDrop(uint64_t client_id,
                                              fbs::ServerCharacterDropT &r) {
  if (CursorGameInstance::cursor_hand_net_.has_value()) {
    if (r.client_id == ServerMain::local_addr_) {
      CursorGameInstance::cursor_hand_net_ = std::nullopt;
      if (CursorGameInstance::cursor_hand_.has_value()) {
        auto &hand = CursorGameInstance::cursor_hand_.value();
        switch (hand.type) {
        case CursorGameInstance::CursorHandType::kEquipment: {
          break;
        }
        case CursorGameInstance::CursorHandType::kPackage: {
          int num = 1;
          switch (r.payload->drop.type) {
          case fbs::ItemUnion_Item: {
            auto item = r.payload->drop.AsItem();
            num = item->item_num;
            break;
          }
          default: {
            break;
          }
          }
          auto &itm = PackageGameInstance::data_[hand.val][hand.sub_val];
          switch (static_cast<ItemEnum>(hand.val)) {
          case ItemEnum::kEquip: {
            itm = std::polymorphic<GameItem>(GameEquipItem{});
            break;
          }
          case ItemEnum::kDeco: {
            itm = std::polymorphic<GameItem>(GameDecoItem{});
            break;
          }
          default: {
            ItemGameInstance::DecItemNum(itm, num);
            auto itm_num = ItemGameInstance::LoadItemNum(itm);
            if (itm_num == 0) {
              itm->id = u"";
            }
            break;
          }
          }
          break;
        }
        default: {
          break;
        }
        }
      } else {
        // meso
        auto item = r.payload->drop.AsItem();
        auto num = item->item_num;
        PackageGameInstance::meso_ -= num;
      }

      CursorGameInstance::cursor_hand_ = std::nullopt;
    }
  }
  HandleServerDt(*r.payload);
  AudioGameInstance::LoadAudio(u"Game.img/DropItem", 0);
}

std::vector<std::unique_ptr<fbs::DropT>>
ServerDropInstance::CreateDts(const std::vector<fbs::DropT> &dts,
                                 uint32_t map_id) {
  std::vector<std::unique_ptr<fbs::DropT>> r;

  auto &gen = RandomGameInstance::gen;
  std::uniform_int_distribution<uint64_t> dist(1, UINT64_MAX);

  const auto &s_fhs = ServerSceneInstance::scenes_.at(map_id).fhs;
  std::flat_map<int32_t, GameFoothold> g_fhs;
  for (const auto &[key, value] : s_fhs) {
    g_fhs.emplace(key, value.fh);
  }
  auto border = MapInfoGameInstance::LoadMrBorder(map_id);
  border.w -= 80;
  auto dts_size = dts.size();
  const auto dt_w = 24;

  int n = 0;
  auto max_h = std::pow(400, 2) / (2 * 800);

  for (auto dt : dts) {
    uint64_t random_id = 0;
    do {
      random_id = dist(gen);
    } while (ServerSceneInstance::scenes_[map_id].drops.contains(random_id));
    dt.random_id = dist(gen);
    int x;
    if (dts_size == 1) {
      x = 0;
    } else {
      float center_offset = (dts_size - 1) / 2.0f;
      x = (int)((n - center_offset) * dt_w);
    }

    dt.x2 = dt.x1 + x;
    dt.y2 = dt.y1 - max_h;

    // 提取判断墙面
    auto ins =
        Physic::FallIntersectPos({dt.x1, dt.y1}, {dt.x2, dt.y2}, g_fhs);
    auto hspeed = (dt.x1 > dt.x2) ? -1 : 1;

    for (const auto &[k, v] : ins) {
      if (!v.fh.k.has_value()) {
        auto check = Physic::FallCollideWall(hspeed, v.fh, g_fhs);
        if (check) {
          dt.x2 = v.pos.x;
          break;
        }
      }
    }

    int32_t tmp_fh;
    uint8_t tmp_page;
    float tmp_hsp = 0;
    float tmp_vsp = 10000;
    SDL_FPoint tmp_fp{dt.x2, dt.y2};
    Physic::Fall(tmp_fp, 100000, tmp_hsp, tmp_vsp, tmp_vsp, tmp_vsp, border,
                 true, true, tmp_fh, tmp_page, g_fhs);
    if (tmp_fp.y == border.h) {
      // 如果这个掉落物会掉落在地图外，则修正x为原始x
      tmp_fp = {dt.x1, dt.y2};
      Physic::Fall(tmp_fp, 100000, tmp_hsp, tmp_vsp, tmp_vsp, tmp_vsp, border,
                   true, true, tmp_fh, tmp_page, g_fhs);
    }
    dt.page = tmp_page;

    dt.x2 = tmp_fp.x;
    dt.y2 = tmp_fp.y;

    r.push_back(std::make_unique<fbs::DropT>(dt));
    n++;
  }
  return r;
}

void ServerDropInstance::HandleServerDropFade(fbs::ServerDropFadeT &r) {
  auto &data = DropGameInstance::data_;
  for (const auto id : r.drop_ids) {
    if (data.contains(id)) {
      data.at(id).type = GameDrop::DropEnum::kFade;
    }
  }
  return;
}

void ServerDropInstance::HandleClientDropFade(fbs::ClientDropFadeT &r) {
  auto map_id = r.map_id;
  auto &drops = ServerSceneInstance::scenes_.at(map_id).drops;
  for (auto id : r.random_id) {
    if (drops.contains(id)) {
      drops.at(id).destroy = Window::dt_now_;
    }
  }
  return;
}

void ServerDropInstance::HandleClientCreateDrop(uint64_t client_id,
                                                     fbs::ClientCreateDropT &r) {
  auto map_id = r.map_id;

  std::vector<fbs::DropT> dt;
  for (auto &p : r.drops) {
    dt.push_back(*p);
  }
  auto dts = CreateDts(dt, map_id);
  for (const auto &dt : dts) {
    SaveDelayDrop(map_id, r.delay, *dt);
  }
}

void ServerDropInstance::HandleServerCreateDrop(uint64_t client_id,
                                                     fbs::ServerCreateDropT &r) {
  for (const auto &dt : r.payload) {
    ServerDropInstance::HandleServerDt(*dt);
  }
  AudioGameInstance::LoadAudio(u"Game.img/DropItem", 0);
}

}  // namespace sdlms
