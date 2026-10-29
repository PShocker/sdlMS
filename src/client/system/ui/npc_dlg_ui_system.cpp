#include "src/client/system/ui/npc_dlg_ui_system.h"

#include "src/client/game/game_npc.h"
#include "src/client/game/game_quest.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/button.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/quest_alarm_ui_system.h"
#include "src/client/system/ui/quest_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/script/script.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

static std::u16string selected;

void NpcDlgUiSystem::ResetPos() {
  auto wh = LoadWh();
  auto& camera = CameraGameInstance::camera_;
  pos_.x = (camera.w - wh.x) / 2;
  pos_.y = (camera.h - wh.y) / 2;
}

void NpcDlgUiSystem::RenderBackgrnd() {
  auto [w, h] = LoadWh();
  static auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/t"));
  SDL_FRect pos_rect{
      pos_.x,
      pos_.y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  static auto c = WzResource::LoadTexture(
      WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/c"));
  static auto s = WzResource::LoadTexture(
      WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/s"));
  pos_rect = {
      pos_.x,
      pos_.y + t->h,
      static_cast<float>(c->w),
      static_cast<float>(h - t->h - s->h),
  };
  SDL_RenderTextureTiled(Window::renderer_, c, nullptr, 1, &pos_rect);

  pos_rect = {
      pos_.x,
      pos_.y + h - s->h,
      static_cast<float>(s->w),
      static_cast<float>(s->h),
  };
  SDL_RenderTexture(Window::renderer_, s, nullptr, &pos_rect);
}

void NpcDlgUiSystem::RenderNpc() {
  auto [w, h] = LoadWh();
  GameNpc npc;
  npc.action = u"stand";
  const auto& camera = CameraGameInstance::camera_;
  npc.pos.x = camera.x + pos_.x + 80;
  npc.pos.y = camera.y + pos_.y + 15 + h / 2;
  npc.ani_index = 0;
  npc.id = npc_id_;
  NpcRenderSystem::RenderNpc(npc);

  //   render bar
  static auto t = WzResource::LoadTexture(
      WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/bar"));
  SDL_FRect pos_rect{
      pos_.x + 80 - t->w / 2,
      pos_.y + 15 + h / 2,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);

  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(255, 255, 255, 255);
  auto npc_name = NpcGameInstance::LoadNpcText(npc.id, u"name");
  w = Freetype::LoadW(npc_name);
  Freetype::DrawLine(npc_name, pos_rect.x + 61 - w / 2, pos_rect.y);
  Freetype::LoadAligned(false);
}

void NpcDlgUiSystem::RenderButton() {
  auto [w, h] = LoadWh();
  std::vector<wz::Node*> buttons_node = {
      WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtClose"),
  };
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18},  //
  };

  auto w0 = w - 72;
  if (index_ < max_index_) {
    buttons_node.push_back(
        WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtNext"));
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    w0 -= 52;
  }
  if (index_ > 0) {
    buttons_node.push_back(
        WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtPrev"));
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
  }
  if (type_ == NpcDlgEnum::kQuestAvaliable && index_ == max_index_) {
    buttons_node.push_back(WzResource::ui_->find(u"UIWindow.img/Quest/BtOK"));
    buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});

    buttons_node.push_back(WzResource::ui_->find(u"UIWindow.img/Quest/BtNo"));
    buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});
  } else {
    buttons_node.push_back(
        WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtOK"));
    buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});
  }

  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];
    rect.x += (int)pos_.x;
    rect.y += (int)pos_.y;
    Button{.node = buttons_node[i],
           .rect = rect,
           .enable = true,
           .focused = CursorGameInstance::cursor_ui_ == Render &&
                      !CursorGameInstance::modal_overlay_}
        .Render();
  }
}

void NpcDlgUiSystem::RenderText() {
  auto dt = Window::dt_now_ - time_;
  int visible_count = int(dt * 0.05);
  switch (type_) {
    case NpcDlgEnum::kQuest:
    case NpcDlgEnum::kQuestAvaliable:
    case NpcDlgEnum::kQuestComplete:
    case NpcDlgEnum::kQuestYes:
    case NpcDlgEnum::kQuestNo:
    case NpcDlgEnum::kQuestStopItem:
    case NpcDlgEnum::kQuestStopLost:
    case NpcDlgEnum::kQuestStopMob:
    case NpcDlgEnum::kQuestStopNpc:
    case NpcDlgEnum::kTalk:
    case NpcDlgEnum::kSelect: {
      Freetype::LoadSize(12);
      Freetype::LoadAligned(true);
      Freetype::LoadColor(0, 0, 0, 255);
      select_mouse_ =
          Freetype::DrawRstr(text_, pos_.x + 165, pos_.y + 30, 330, 1.3,
                             std::nullopt, select_, false, visible_count)
              .select;
      break;
    }
    default: {
      break;
    }
  }
}

void NpcDlgUiSystem::RenderList() {
  if (type_ != NpcDlgEnum::kQuest) {
    return;
  }
  if (index_ != 0) {
    return;
  }
  auto [w, h] = LoadWh();

  static auto t0 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/list0"));
  static auto t1 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/list1"));
  static auto t2 = WzResource::LoadTexture(
      WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/list3"));

  auto avaliable_quest = NpcGameInstance::LoadAvaliableQuest(npc_id_);
  auto progress_quest = NpcGameInstance::LoadProgressQuest(npc_id_);
  auto progress_complete_quest =
      NpcGameInstance::LoadProgressCompleteQuest(npc_id_);

  std::ranges::sort(progress_quest);
  std::ranges::sort(progress_complete_quest);

  std::vector<std::u16string> result;
  std::ranges::set_difference(progress_quest, progress_complete_quest,
                              std::back_inserter(result));
  progress_quest = result;

  const auto& mouse_pos = Window::mouse_pos_;
  selected = u"";
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(128, 0, 128, 255);
  auto lh = Freetype::LoadLh();

  const auto render_q = [&](SDL_Texture* t, std::vector<std::u16string>& q,
                            float& y, std::u16string type) {
    if (q.empty()) {
      return;
    }
    SDL_FRect pos_rect{
        static_cast<float>((int)pos_.x + 165),
        (int)pos_.y + y,
        static_cast<float>(t->w),
        static_cast<float>(t->h),
    };
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
    y += 20;
    for (int i = 0; i < q.size(); i++) {
      auto quest_node = QuestGameInstance::LoadQuestNode(q[i]);
      auto name_node = quest_node->find(u"QuestInfo/name");
      auto name = static_cast<wz::Property<std::u16string>*>(name_node)->get();
      auto lv_node = quest_node->find(u"Check/0/lvmin");
      auto lv = 0;
      if (lv_node) {
        lv = static_cast<wz::Property<int>*>(lv_node)->get();
      }
      auto lv2 = std::to_string(lv);
      std::u16string lv3{lv2.begin(), lv2.end()};
      name = u"(Lv." + lv3 + u") " + name + u" (" + type + u")";
      auto text_x = pos_rect.x + 12;
      auto text_y = pos_.y + y;
      Freetype::DrawLine(name, text_x, text_y);
      auto text_w = Freetype::LoadW(name);
      SDL_FRect r{
          text_x,
          text_y,
          text_w,
          lh,
      };
      if (SDL_PointInRectFloat(&mouse_pos, &r)) {
        SDL_SetRenderDrawColor(Window::renderer_, 128, 0, 128, 255);
        SDL_RenderLine(Window::renderer_, text_x, text_y + lh, text_x + text_w,
                       text_y + lh);
        selected = q[i];
      }
      y += lh;
    }
    y += 12;
  };
  auto y = Freetype::LoadCh(text_, 330, 1.3) + 40;
  render_q(t2, progress_complete_quest, y, u"Complete");
  render_q(t1, avaliable_quest, y, u"Avaliable");
  render_q(t0, progress_quest, y, u"Progress");

  // script
  y += 10;
  script_id_ = u"";
  static auto t_etc = WzResource::LoadTexture(
      WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/list2"));
  auto scp_id = NpcGameInstance::LoadNpcScript(npc_id_);
  if (!scp_id.empty()) {
    SDL_FRect pos_rect{
        static_cast<float>((int)pos_.x + 165),
        (int)pos_.y + y,
        static_cast<float>(t_etc->w),
        static_cast<float>(t_etc->h),
    };
    SDL_RenderTexture(Window::renderer_, t_etc, nullptr, &pos_rect);
    auto text_x = pos_rect.x + 12;
    auto text_y = pos_rect.y + 16;
    std::u16string str;
    str = u"(" + scp_id + u")" + u" Talk to " +
          NpcGameInstance::LoadNpcText(npc_id_, u"name");
    Freetype::DrawLine(str, text_x, text_y);
    auto text_w = Freetype::LoadW(str);
    SDL_FRect r{
        text_x,
        text_y,
        text_w,
        lh,
    };
    if (SDL_PointInRectFloat(&mouse_pos, &r)) {
      SDL_SetRenderDrawColor(Window::renderer_, 128, 0, 128, 255);
      SDL_RenderLine(Window::renderer_, text_x, text_y + lh, text_x + text_w,
                     text_y + lh);
      script_id_ = scp_id;
    }
  }
}

void NpcDlgUiSystem::RenderObtain() {
  if (type_ != NpcDlgEnum::kQuestComplete) {
    return;
  }
  if (index_ != max_index_) {
    return;
  }
  static auto t =
      WzResource::LoadTexture(WzResource::ui_->find(u"QuestIcon.img/4/0"));
  auto y = Freetype::LoadRh(text_, 330, 1.3, std::nullopt) + 40;
  SDL_FRect pos_rect{
      static_cast<float>((int)pos_.x + 165),
      (int)pos_.y + y,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  pos_rect.y += 20;
  if (act_exp_ != 0) {
    static auto t2 =
        WzResource::LoadTexture(WzResource::ui_->find(u"QuestIcon.img/8/0"));
    pos_rect.w = t2->w;
    pos_rect.h = t2->h;
    SDL_RenderTexture(Window::renderer_, t2, nullptr, &pos_rect);
    Freetype::LoadSize(12);
    Freetype::LoadAligned(true);
    Freetype::LoadColor(0, 0, 0, 255);
    auto tmp = std::to_string(act_exp_);
    Freetype::DrawLine({tmp.begin(), tmp.end()}, pos_rect.x + 22, pos_rect.y);
    pos_rect.y += t2->h + 8;
  }
  if (act_meso_ != 0) {
    static auto t3 =
        WzResource::LoadTexture(WzResource::ui_->find(u"QuestIcon.img/7/0"));
    pos_rect.y += 3;
    pos_rect.w = t3->w;
    pos_rect.h = t3->h;
    SDL_RenderTexture(Window::renderer_, t3, nullptr, &pos_rect);
    Freetype::LoadSize(12);
    Freetype::LoadAligned(true);
    Freetype::LoadColor(0, 0, 0, 255);
    auto tmp = std::to_string(act_meso_);
    Freetype::DrawLine({tmp.begin(), tmp.end()}, pos_rect.x + 22, pos_rect.y);
    pos_rect.y += t3->h + 8;
  }
  if (!act_random_item_.empty()) {
    static auto t =
        WzResource::LoadTexture(WzResource::ui_->find(u"QuestIcon.img/5/0"));
    pos_rect.w = t->w;
    pos_rect.h = t->h;
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
    std::u16string name = u"Random Item";
    Freetype::DrawLine(name, pos_rect.x + t->w + 4, pos_rect.y + t->h - 18);
    pos_rect.y += t->h + 8;
  }
  if (!act_item_.empty()) {
    for (auto [k, v] : act_item_) {
      if (v <= 0) {
        continue;
      }
      SDL_Texture* t;
      std::u16string name;
      if (ItemGameInstance::CheckItem(k)) {
        auto info = ItemGameInstance::LoadItemInfo(k, 0);
        t = WzResource::LoadTexture(info->get_child(u"icon"));
        name = ItemGameInstance::LoadItemText(k, u"name");
      } else {
        auto info = EquipGameInstance::LoadEquipInfo(k);
        t = WzResource::LoadTexture(info->get_child(u"icon"));
        name = EquipGameInstance::LoadEquipName(k);
      }
      pos_rect.w = t->w;
      pos_rect.h = t->h;
      SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
      Freetype::LoadSize(12);
      Freetype::LoadAligned(true);
      Freetype::LoadColor(0, 0, 0, 255);
      auto tmp = std::to_string(v);
      name = name + u" x" + std::u16string{tmp.begin(), tmp.end()};
      Freetype::DrawLine(name, pos_rect.x + t->w + 4, pos_rect.y + t->h - 18);
      pos_rect.y += t->h + 8;
    }
  }
}

bool NpcDlgUiSystem::Render() {
  RenderBackgrnd();
  RenderNpc();
  RenderButton();
  RenderText();
  RenderList();
  RenderObtain();
  return true;
}

SDL_FPoint NpcDlgUiSystem::LoadWh() {
  Freetype::LoadSize(12);
  auto h = Freetype::LoadRh(text_, 330, 1.3, std::nullopt);
  h = h + 130;
  if (type_ == NpcDlgEnum::kQuest && index_ == 0) {
    auto avaliable_quest = NpcGameInstance::LoadAvaliableQuest(npc_id_);
    auto progress_quest = NpcGameInstance::LoadProgressQuest(npc_id_);
    auto progress_complete_quest =
        NpcGameInstance::LoadProgressCompleteQuest(npc_id_);

    std::ranges::sort(progress_quest);
    std::ranges::sort(progress_complete_quest);

    std::vector<std::u16string> result;
    std::ranges::set_difference(progress_quest, progress_complete_quest,
                                std::back_inserter(result));
    progress_quest = result;
    Freetype::LoadSize(12);
    auto lh = Freetype::LoadLh();
    for (const auto& v :
         {avaliable_quest, progress_quest, progress_complete_quest}) {
      if (!v.empty()) {
        h += v.size() * lh;
        h += 18;
      }
    }
    auto scp_id = NpcGameInstance::LoadNpcScript(npc_id_);
    if (!scp_id.empty()) {
      h += 36;
    }
  } else if (type_ == NpcDlgEnum::kQuestComplete && index_ == max_index_) {
    if (act_exp_ != 0) {
      h += 22;
    }
    if (!act_random_item_.empty()) {
      h += 27;
    }
    for (auto [k, v] : act_item_) {
      if (v > 0) {
        h += 32;
      }
    }
    if (act_meso_ != 0) {
      h += 22;
    }
  }
  h = std::max((int)h, 190);

  return {529, h};
}

void NpcDlgUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    ResetPos();

    KeyboardInputSystem::Reset();

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void NpcDlgUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
}

void NpcDlgUiSystem::Toggle() {
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

void NpcDlgUiSystem::EventButtonClose() {
  Close();
}

void NpcDlgUiSystem::EventButtonOk() {
  if (type_ == NpcDlgEnum::kQuestComplete && index_ == max_index_) {
    auto back_meso = PackageGameInstance::meso_;
    auto back_data = PackageGameInstance::data_;
    if (!act_random_item_.empty()) {
      auto& r = RandomGameInstance::gen_;
      std::uniform_int_distribution<int> dist(0, act_random_item_.size() - 1);
      auto it = act_random_item_.begin();
      std::advance(it, dist(r));
      act_item_.insert(*it);
    }
    for (auto [k, v] : act_item_) {
      if (v > 0) {
        std::polymorphic<GameItem> item;
        if (ItemGameInstance::CheckItem(k)) {
          item = ItemGameInstance::LoadItem(k, v);
        } else {
          item = EquipGameInstance::LoadItem(k);
        }
        if (!PackageGameInstance::AddNewItem(item)) {
          NoticeUiSystem::OpenNoSpace(item->type);
          PackageGameInstance::meso_ = back_meso;
          PackageGameInstance::data_ = back_data;
          Close();
          return;
        }
      } else {
        auto item = PackageGameInstance::LoadItem(k);
        ItemGameInstance::DecItemNum(*item, -v);
      }
    }
    PackageGameInstance::AddMeso(act_meso_);
    CharacterStatGameInstance::AddExp(act_exp_);
    if (StartNextQuest()) {
      return;
    }
    QuestGameInstance::CompleteQuest(quest_id_);
    QuestAlarmUiSystem::CompleteQuest(quest_id_);
    QuestUiSystem::CompleteQuest(quest_id_);
    auto quest_node = QuestGameInstance::LoadQuestNode(quest_id_);
    if (auto n = quest_node->find(u"Say/1/yes/0"); n != nullptr) {
      time_ = Window::dt_now_;
      text_ = TextGameInstance::LoadRstr(n);
      index_ = 0;
      max_index_ = n->children_count() - 1;
      return;
    }
  }
  Close();
}

void NpcDlgUiSystem::EventButtonUpdate() {
  if (cb_) {
    cb_();
    ResetPos();
    return;
  }
  if (!script_id_.empty()) {
    Script::fns().at(script_id_)(nullptr);
    ResetPos();
    return;
  }
  if (quest_id_.empty()) {
    return;
  }
  // 根据 type 决定后缀，统一拼接路径
  std::u16string suffix;
  switch (type_) {
    case NpcDlgEnum::kQuestComplete:
    case NpcDlgEnum::kQuest:
      suffix = u"";
      break;
    case NpcDlgEnum::kQuestYes:
      suffix = u"/yes";
      break;
    case NpcDlgEnum::kQuestStopItem:
      suffix = u"/item";
      break;
    case NpcDlgEnum::kQuestStopLost:
      suffix = u"/lost";
      break;
    case NpcDlgEnum::kQuestStopNpc:
      suffix = u"/npc";
      break;
    default:
      suffix = u"";
  }
  auto node = QuestGameInstance::LoadQuestNode(quest_id_);
  if (node) {
    node = node->find(u"Say/" + quest_index_ + suffix);
  }
  if (node != nullptr) {
    text_ = TextGameInstance::LoadRstr(node->get_child(std::to_string(index_)));
  }
  ResetPos();
  time_ = Window::dt_now_;
}

void NpcDlgUiSystem::EventButtonPrev() {
  selected = u"";
  --index_;
  EventButtonUpdate();
}

void NpcDlgUiSystem::EventButtonNext() {
  switch (type_) {
    case NpcDlgEnum::kSelect: {
      if (cb_) {
        cb_();
        return;
      }
      auto node = QuestGameInstance::LoadQuestNode(quest_id_);
      node = node->find(u"Say/" + quest_index_ + u"/stop/0/answer");
      auto ans = static_cast<wz::Property<int>*>(node)->get() - 1;
      auto st = select_;
      if (select_mouse_ != -1) {
        st = select_mouse_;
      }
      if (ans == st) {
        type_ = NpcDlgEnum::kQuestComplete;
        node = QuestGameInstance::LoadQuestNode(quest_id_);
        node = node->find(u"Say/" + quest_index_);
        node = node->get_child(std::to_string(index_ + 1));
        time_ = Window::dt_now_;
        text_ = TextGameInstance::LoadRstr(node);
        index_ = 0;
        max_index_ = 0;

        act_meso_ = QuestGameInstance::LoadQuestActMeso(quest_id_);
        act_item_ = QuestGameInstance::LoadQuestActItem(quest_id_);
        act_exp_ = QuestGameInstance::LoadQuestActExp(quest_id_);
      } else {
        type_ = NpcDlgEnum::kTalk;
        node = QuestGameInstance::LoadQuestNode(quest_id_);
        node = node->find(u"Say/" + quest_index_ + u"/stop");
        node = node->get_child(std::to_string(index_));
        node = node->get_child(std::to_string(st));
        time_ = Window::dt_now_;
        text_ = TextGameInstance::LoadRstr(node);
        index_ = 0;
        max_index_ = 0;
      }
      break;
    }
    default: {
      selected = u"";
      ++index_;
      EventButtonUpdate();
      break;
    }
  }
}

void NpcDlgUiSystem::EventList() {
  if (type_ != NpcDlgEnum::kQuest) {
    return;
  }
  if (index_ != 0) {
    return;
  }
  if (type_ == NpcDlgEnum::kQuest && !script_id_.empty()) {
    Script::fns().at(script_id_)(nullptr);
    return;
  }
  if (selected.empty()) {
    return;
  }
  script_id_ = u"";
  quest_id_ = selected;

  auto progress = QuestGameInstance::LoadQuestProgress(selected);
  auto tmp = std::to_string(progress);
  quest_index_ = {tmp.begin(), tmp.end()};
  auto progress_complete = NpcGameInstance::LoadProgressCompleteQuest(npc_id_);

  wz::Node* node = nullptr;
  if (std::ranges::contains(progress_complete, quest_id_)) {
    // 待完成
    node = QuestGameInstance::LoadQuestNode(quest_id_);
    if (auto n = node->find(u"Check/" + quest_index_ + u"/endscript");
        n != nullptr) {
      auto spt = static_cast<wz::Property<std::u16string>*>(n)->get();
      Script::fns().at(spt)(nullptr);
      script_id_ = spt;
      return;
    }
    if (node->find(u"Say/" + quest_index_ + u"/ask")) {
      text_ = TextGameInstance::LoadRstr(
          node->find(u"Say/" + quest_index_ + u"/0"));
      type_ = NpcDlgEnum::kSelect;
      select_ = 0;
      max_select_ = -1;
      for (size_t i = 0; i + 1 < text_.size(); ++i) {
        if (text_[i] == u'#' && text_[i + 1] == u'L') {
          ++max_select_;
          i = i + 3;
        }
      }
      auto n = node->find(u"Say/" + quest_index_);
      auto m = n->children;
      m.erase(u"ask");
      m.erase(u"stop");
      index_ = 0;
      max_index_ = m.size() - 1;
      time_ = Window::dt_now_;
      select_mouse_ = -1;
      return;
    }

    type_ = NpcDlgEnum::kQuestComplete;
    node = node->find(u"Say/" + quest_index_);
    auto child = node->children;
    child.erase(u"yes");
    child.erase(u"no");
    child.erase(u"stop");
    child.erase(u"lost");
    max_index_ = child.size() - 1;

    act_meso_ = QuestGameInstance::LoadQuestActMeso(quest_id_);
    act_item_ = QuestGameInstance::LoadQuestActItem(quest_id_);
    act_exp_ = QuestGameInstance::LoadQuestActExp(quest_id_);
    act_random_item_ = QuestGameInstance::LoadQuestActRandomItem(quest_id_);
    if ((act_meso_ == 0 && act_exp_ == 0) || max_index_ == -1) {
      max_index_ = 0;
      index_ = max_index_;
      EventButtonOk();
      return;
    }

  } else if (QuestGameInstance::progress_quests_.contains(quest_id_)) {
    // 判断是否是进行中的任务
    auto& q = QuestGameInstance::progress_quests_.at(quest_id_);
    if (!q.item_bool) {
      type_ = NpcDlgEnum::kQuestStopItem;
      node = QuestGameInstance::LoadQuestNode(selected);
      node = node->find(u"Say/" + quest_index_ + u"/stop/npc");
    } else if (!q.mob_bool) {
      type_ = NpcDlgEnum::kQuestStopItem;
      node = QuestGameInstance::LoadQuestNode(selected);
      node = node->find(u"Say/" + quest_index_ + u"/stop/mob");
    } else {
      type_ = NpcDlgEnum::kQuestStopNpc;
      node = QuestGameInstance::LoadQuestNode(selected);
      node = node->find(u"Say/" + quest_index_ + u"/stop/item");
    }
    if (node) {
      max_index_ = node->children_count();
    }
  } else {
    type_ = NpcDlgEnum::kQuestAvaliable;
    node = QuestGameInstance::LoadQuestNode(selected);
    if (auto n = node->find(u"Check/0/startscript"); n != nullptr) {
      auto spt = static_cast<wz::Property<std::u16string>*>(n)->get();
      Script::fns().at(spt)(nullptr);
      script_id_ = spt;
      return;
    }
    node = node->find(u"Say/" + quest_index_);
    auto child = node->children;
    child.erase(u"yes");
    child.erase(u"no");
    child.erase(u"stop");
    child.erase(u"lost");
    max_index_ = child.size() - 1;
  }
  if (node != nullptr && node->get_child(u"0")) {
    index_ = 0;
    text_ = TextGameInstance::LoadRstr(node->get_child(u"0"));
    time_ = Window::dt_now_;
    ResetPos();
  }
}

void NpcDlgUiSystem::EventButtonQuestYes() {
  if (!script_id_.empty()) {
    index_++;
    Script::fns().at(script_id_)(nullptr);
    return;
  }
  act_item_ = QuestGameInstance::LoadQuestActItem(quest_id_);
  auto back_data = PackageGameInstance::data_;
  for (auto [k, v] : act_item_) {
    if (v > 0) {
      std::polymorphic<GameItem> item;
      if (ItemGameInstance::CheckItem(k)) {
        item = ItemGameInstance::LoadItem(k, v);
      } else {
        item = EquipGameInstance::LoadItem(k);
      }
      if (!PackageGameInstance::AddNewItem(item)) {
        NoticeUiSystem::OpenNoSpace(item->type);
        PackageGameInstance::data_ = back_data;
        Close();
        return;
      }
    } else {
      auto item = PackageGameInstance::LoadItem(k);
      ItemGameInstance::DecItemNum(*item, -v);
    }
  }
  // accept quest
  QuestGameInstance::AcceptQuest(quest_id_);
  QuestAlarmUiSystem::AcceptQuest(quest_id_);
  static wz::Node* yes_node;
  yes_node = QuestGameInstance::LoadQuestNode(quest_id_);
  yes_node = yes_node->find(u"Say/" + quest_index_ + u"/yes");
  if (yes_node == nullptr) {
    Close();
    return;
  } else {
    type_ = NpcDlgEnum::kQuestYes;
    index_ = 0;
    max_index_ = yes_node->children_count() - 1;
    auto node = yes_node->get_child(u"0");
    text_ = TextGameInstance::LoadRstr(node);
    time_ = Window::dt_now_;
  }
}

void NpcDlgUiSystem::EventButtonQuestNo() {
  static wz::Node* no_node;
  no_node = QuestGameInstance::LoadQuestNode(quest_id_);
  no_node = no_node->find(u"Say/" + quest_index_ + u"/no");
  if (no_node == nullptr) {
    Close();
    return;
  } else {
    type_ = NpcDlgEnum::kQuestNo;
    index_ = 0;
    max_index_ = no_node->children_count() - 1;
    auto node = no_node->get_child(u"0");
    text_ = TextGameInstance::LoadRstr(node);
    time_ = Window::dt_now_;
  }
}

bool NpcDlgUiSystem::EventButton(SDL_Event* event) {
  auto [w, h] = LoadWh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18},  //
  };
  std::vector<std::function<void()>> func = {
      EventButtonClose,
  };

  auto w0 = w - 72;
  if (index_ < max_index_) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    func.push_back(EventButtonNext);
    w0 -= 52;
  }
  if (index_ > 0) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    func.push_back(EventButtonPrev);
  }
  if (index_ == max_index_ && cb_ == nullptr &&
      type_ == NpcDlgEnum::kQuestAvaliable) {
    buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});
    func.push_back(EventButtonQuestYes);

    buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});
    func.push_back(EventButtonQuestNo);
  } else {
    buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});
    func.push_back(EventButtonOk);
  }

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      func[i]();
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      return true;
    }
  }
  return false;
}

bool NpcDlgUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void NpcDlgUiSystem::EventMotionBtAudio(SDL_Event* event) {
  if (CursorGameInstance::cursor_ui_ != Render) {
    return;
  }
  auto [w, h] = LoadWh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{10, h - 25, 85, 18},  //
  };
  auto w0 = w - 72;
  if (index_ < max_index_) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
    w0 -= 52;
  }
  if (index_ > 0) {
    buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});
  }
  if (index_ == max_index_ && cb_ == nullptr &&
      type_ == NpcDlgEnum::kQuestAvaliable) {
    buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});
    buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});
  } else {
    buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});
  }
  static int8_t bt_audio = -1;
  auto& mouse_pos = Window::mouse_pos_;
  int8_t bt_in = -1;
  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos_.x;
    pos_rect.y += (int)pos_.y;
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      bt_in = i;
      break;
    }
  }
  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
  }
  bt_audio = bt_in;
}

bool NpcDlgUiSystem::Event(SDL_Event* event) {
  bool r = false;
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      switch (scan_code) {
        case SDL_SCANCODE_UP: {
          if (type_ == NpcDlgEnum::kSelect) {
            select_--;
            select_ = std::max((int8_t)0, select_);
          }
          break;
        }
        case SDL_SCANCODE_DOWN: {
          if (type_ == NpcDlgEnum::kSelect) {
            select_++;
            select_ = std::min(select_, max_select_);
          }
          break;
        }
        case SDL_SCANCODE_ESCAPE: {
          EventButtonClose();
          return false;
          break;
        }
        case SDL_SCANCODE_RETURN: {
          if (index_ == max_index_ && cb_ == nullptr &&
              type_ == NpcDlgEnum::kQuestAvaliable) {
            EventButtonQuestYes();
            return false;
          }
          if (type_ == NpcDlgEnum::kQuestAvaliable && index_ < max_index_) {
            EventButtonNext();
            return false;
          }
          if (type_ == NpcDlgEnum::kQuestComplete) {
            EventButtonOk();
            return false;
          }
          EventButtonClose();
          return false;
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          time_ = UINT64_MAX;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          time_ = UINT64_MAX;
          EventList();
          EventButton(event);
          if (type_ == NpcDlgEnum::kSelect && select_mouse_ >= 0) {
            EventButtonNext();
          }
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventMotionBtAudio(event);
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

bool NpcDlgUiSystem::StartNextQuest() {
  auto node = QuestGameInstance::LoadQuestNode(quest_id_);
  if (auto n = node->find(u"Act/" + quest_index_ + u"/nextQuest");
      n != nullptr) {
    QuestGameInstance::CompleteQuest(selected);
    QuestAlarmUiSystem::CompleteQuest(selected);
    QuestUiSystem::CompleteQuest(quest_id_);
    auto nq = static_cast<wz::Property<int>*>(n)->get();
    auto tmp = std::to_string(nq);
    selected = std::u16string{tmp.begin(), tmp.end()} + u".img";
    type_ = NpcDlgEnum::kQuest;
    EventList();
    return true;
  }
  return false;
}

}  // namespace sdlms
