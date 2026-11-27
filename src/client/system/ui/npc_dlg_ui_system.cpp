#include "src/client/system/ui/npc_dlg_ui_system.h"

#include "src/client/game/game_npc.h"
#include "src/client/game/game_quest.h"
#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/equip_game_instance.h"
#include "src/client/game_instance/item_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/npc_game_instance.h"
#include "src/client/game_instance/package_game_instance.h"
#include "src/client/game_instance/quest_game_instance.h"
#include "src/client/game_instance/random_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/game_instance/text_game_instance.h"
#include "src/client/system/input/keyboard_input_system.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/render/npc_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/component/ui_component.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/system/ui/quest_alarm_ui_system.h"
#include "src/client/system/ui/quest_ui_system.h"
#include "src/client/system_instance/scene_system_instance.h"
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
#include <limits>
#include <optional>
#include <string>
#include <vector>


namespace sdlms {


static std::u16string selected;


namespace {


// 运行 WZ 数据里指定名字的任务脚本。数据中引用了尚未实现的脚本名时返回 false，

// 让调用方退回普通的接任务／交任务流程，而不是抛 std::out_of_range 崩掉。

bool RunScript(const std::u16string& name) {
  auto& fns = Script::fns();

  auto it = fns.find(name);

  if (it == fns.end()) {
    return false;
  }

  it->second(nullptr);

  return true;
}


}  // namespace


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


UIComponents& NpcDlgUiSystem::GetComponent() {
  components_.clear();

  auto [w, h] = LoadWh();


  // 脚本对话的确认结果页只有 OK：不显示关闭按钮，也不允许 Esc 退回。

  const bool script_result =

      type_ == NpcDlgEnum::kScript && buttons_ == NpcDlgButton::kOk;


  std::vector<wz::Node*> buttons_node;

  std::vector<SDL_FRect> buttons_rect;

  std::vector<Button::Callback> callbacks;


  if (!script_result) {
    buttons_node.push_back(

        WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtClose"));

    buttons_rect.push_back(SDL_FRect{10, h - 25, 85, 18});

    callbacks.push_back(EventButtonClose);
  }


  if (type_ == NpcDlgEnum::kScript) {
    if (buttons_ == NpcDlgButton::kYesNo) {
      buttons_node.push_back(

          WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtYes"));

      buttons_rect.push_back(SDL_FRect{w - 108, h - 25, 46, 18});

      callbacks.push_back(EventButtonYes);


      buttons_node.push_back(

          WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtNo"));

      buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});

      callbacks.push_back(EventButtonNo);

    } else {
      auto w0 = w - 72;

      if (buttons_ == NpcDlgButton::kDefault) {
        if (index_ < max_index_) {
          buttons_node.push_back(

              WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtNext"));

          buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});

          callbacks.push_back(EventButtonNext);

          w0 -= 52;
        }

        if (index_ > 0) {
          buttons_node.push_back(

              WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtPrev"));

          buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});

          callbacks.push_back(EventButtonPrev);
        }
      }

      buttons_node.push_back(

          WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtOK"));

      buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});

      callbacks.push_back(EventButtonOk);
    }

  } else {
    auto w0 = w - 72;

    if (index_ < max_index_) {
      buttons_node.push_back(

          WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtNext"));

      buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});

      callbacks.push_back(EventButtonNext);

      w0 -= 52;
    }

    if (index_ > 0) {
      buttons_node.push_back(

          WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtPrev"));

      buttons_rect.push_back(SDL_FRect{w0, h - 76, 46, 18});

      callbacks.push_back(EventButtonPrev);
    }

    if (index_ == max_index_ && cb_ == nullptr &&

        type_ == NpcDlgEnum::kQuestAvaliable) {
      buttons_node.push_back(WzResource::ui_->find(u"UIWindow.img/Quest/BtOK"));

      buttons_rect.push_back(SDL_FRect{w - 128, h - 25, 57, 17});

      callbacks.push_back(EventButtonQuestYes);


      buttons_node.push_back(WzResource::ui_->find(u"UIWindow.img/Quest/BtNo"));

      buttons_rect.push_back(SDL_FRect{w - 66, h - 25, 57, 17});

      callbacks.push_back(EventButtonQuestNo);

    } else {
      buttons_node.push_back(

          WzResource::ui_->find(u"UtilDlgEx.img/UtilDlgEx/BtOK"));

      buttons_rect.push_back(SDL_FRect{w - 56, h - 25, 46, 18});

      callbacks.push_back(EventButtonOk);
    }
  }


  for (size_t i = 0; i < buttons_node.size(); ++i) {
    SDL_FRect rect = buttons_rect[i];

    rect.x += (int)pos_.x;

    rect.y += (int)pos_.y;

    components_.push_back(

        Button{.node = buttons_node[i],

               .rect = rect,

               .callback = callbacks[i],

               .enable = true,

               .focused = CursorGameInstance::cursor_ui_ == Render});
  }

  return components_;
}


void NpcDlgUiSystem::RenderText() {
  auto dt = Window::dt_now_ - time_;

  // 鼠标点击�?time_ 会被设成 UINT64_MAX（立即显示全部文字）�?

  // 这里不能直接做无符号减法，否�?visible_count 会下溢�?

  int visible_count = std::numeric_limits<int>::max();

  if (time_ != UINT64_MAX) {
    visible_count = static_cast<int>(dt * 0.05);
  }

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

    case NpcDlgEnum::kScript:

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

  render_q(t2, progress_complete_quest, y,

           WzResource::LoadMSString(u"questStatusComplete", u"Complete"));

  render_q(t1, avaliable_quest, y,

           WzResource::LoadMSString(u"questStatusAvailable", u"Avaliable"));

  render_q(t0, progress_quest, y,

           WzResource::LoadMSString(u"questStatusProgress", u"Progress"));


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

    str = u"(" + scp_id + u")" +

          WzResource::LoadMSString(u"talkTo", u"  Talk to ") +

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

    auto num = TextGameInstance::FormatWithCommas(act_exp_);

    auto text = TextGameInstance::LoadU16str(num.c_str()) +

                WzResource::LoadMSString(u"expSuffix", u"  EXP");

    Freetype::DrawLine(text, pos_rect.x + 22, pos_rect.y);

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

    std::string num;

    if (act_meso_ < 0) {
      num = "-" + TextGameInstance::FormatWithCommas(

                      static_cast<uint64_t>(-static_cast<int64_t>(act_meso_)));

    } else {
      num = TextGameInstance::FormatWithCommas(

          static_cast<uint64_t>(act_meso_));
    }

    auto text = TextGameInstance::LoadU16str(num.c_str()) +

                WzResource::LoadMSString(u"mesoSuffix", u"  Meso");

    Freetype::DrawLine(text, pos_rect.x + 22, pos_rect.y);

    pos_rect.y += t3->h + 8;
  }

  if (!act_random_item_.empty()) {
    static auto t =

        WzResource::LoadTexture(WzResource::ui_->find(u"QuestIcon.img/5/0"));

    pos_rect.w = t->w;

    pos_rect.h = t->h;

    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);

    std::u16string name =

        WzResource::LoadMSString(u"randomItem", u"Random Item");

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

  RenderUI();

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
  // 脚本结果页是不可回退的确认页：只能点 OK 继续。

  if (type_ == NpcDlgEnum::kScript && buttons_ == NpcDlgButton::kOk) {
    return;
  }

  Close();
}


void NpcDlgUiSystem::EventButtonYes() {
  if (type_ != NpcDlgEnum::kScript) {
    return;
  }

  if (on_yes_) {
    on_yes_();

  } else {
    Close();
  }
}


void NpcDlgUiSystem::EventButtonNo() {
  if (type_ != NpcDlgEnum::kScript) {
    return;
  }

  if (on_no_) {
    on_no_();

  } else {
    Close();
  }
}


bool NpcDlgUiSystem::SettleAct() {
  auto back_meso = PackageGameInstance::meso_;

  auto back_data = PackageGameInstance::data_;

  auto back_fame = CharacterGameInstance::self_.fame;

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

        CharacterGameInstance::self_.fame = back_fame;

        return false;
      }

    } else {
      auto item = PackageGameInstance::LoadItem(k);

      ItemGameInstance::DecItemNum(*item, -v);
    }
  }

  PackageGameInstance::AddMeso(act_meso_);

  CharacterStatGameInstance::AddExp(act_exp_);

  // Act/<idx>/pop：095 CQuest::LoadReward 的人气奖励。
  CharacterGameInstance::self_.fame += act_pop_;

  // 人气是角色数据的一部分，改完要让同地图玩家收到新的 CharacterT，
  // 否则别人的角色信息面板一直是旧人气。
  if (act_pop_ != 0) {
    CharacterLogicSystem::cct_.map_id = SceneSystemInstance::map_id_;
  }

  // Act/<idx>/skill：095 的 learnSkill，只升不降。
  if (!act_skill_.empty()) {
    for (auto [ski_id, lv] : act_skill_) {
      auto tmp = std::to_string(ski_id);
      std::u16string ski_str{tmp.begin(), tmp.end()};
      const int max_lv = SkillGameInstance::LoadSkiMaxLvl(ski_str);
      const int target = std::min<int>(lv, std::max(0, max_lv));
      if (target > JobSkillGameInstance::LoadSkiLevel(ski_str)) {
        JobSkillGameInstance::skill_point_[ski_id] =
            static_cast<uint8_t>(target);
      }
    }
    // 被动技能等级变化后重算属性。
    CharacterStatGameInstance::Update();
  }

  return true;
}


bool NpcDlgUiSystem::FinishQuest() {
  // 脚本接管了交任务流程，这里自己结算 Act 奖励并把任务标记完成。

  // 不自动推进 nextQuest：脚本对话结束后玩家再找 NPC 接下一环即可。

  if (quest_id_.empty() ||

      !QuestGameInstance::progress_quests_.contains(quest_id_)) {
    return false;
  }

  act_meso_ = QuestGameInstance::LoadQuestActMeso(quest_id_);

  act_item_ = QuestGameInstance::LoadQuestActItem(quest_id_);

  act_exp_ = QuestGameInstance::LoadQuestActExp(quest_id_);

  act_random_item_ = QuestGameInstance::LoadQuestActRandomItem(quest_id_);

  act_pop_ = QuestGameInstance::LoadQuestActPop(quest_id_);

  act_skill_ = QuestGameInstance::LoadQuestActSkill(quest_id_);

  if (!SettleAct()) {
    Close();

    return false;
  }

  QuestGameInstance::CompleteQuest(quest_id_);

  QuestAlarmUiSystem::CompleteQuest(quest_id_);

  QuestUiSystem::CompleteQuest(quest_id_);

  return true;
}


void NpcDlgUiSystem::EventButtonOk() {
  if (type_ == NpcDlgEnum::kScript) {
    if (on_ok_) {
      on_ok_();

    } else {
      Close();
    }

    return;
  }

  // 选项列表（例如二转职业选择）用 OK 确认当前选项

  if (type_ == NpcDlgEnum::kSelect) {
    if (cb_) {
      cb_();

      return;
    }

    // 没有脚本回调的 ask 问答：OK 同样确认当前选项。
    ConfirmSelect();

    return;
  }

  // ask 答错后的提示页：OK 回到问题页重新选择。
  if (type_ == NpcDlgEnum::kTalk && select_retry_) {
    select_retry_ = false;

    type_ = NpcDlgEnum::kSelect;

    text_ = select_text_;

    select_ = 0;

    select_mouse_ = -1;

    index_ = 0;

    max_index_ = 0;

    time_ = Window::dt_now_;

    return;
  }

  // OK 直接确认/关闭，不再先翻页�?

  if (type_ == NpcDlgEnum::kQuestComplete) {
    // 095：结算前重新校验交任务需求（对话期间物品可能变化），
    // 不满足则先不结算，玩家可再次对话。
    if (!QuestGameInstance::CheckCompleteDemand(quest_id_).empty()) {
      Close();

      return;
    }

    if (!SettleAct()) {
      Close();

      return;
    }

    if (StartNextQuest()) {
      return;
    }

    QuestGameInstance::CompleteQuest(quest_id_);

    QuestAlarmUiSystem::CompleteQuest(quest_id_);

    QuestUiSystem::CompleteQuest(quest_id_);

    auto quest_node = QuestGameInstance::LoadQuestNode(quest_id_);

    if (auto n = quest_node->find(u"Say/" + quest_index_ + u"/yes/0");
        n != nullptr) {
      // 奖励已结算，这里只显示完成对白；改为 kQuestYes�?

      // 避免下一�?OK 又进�?kQuestComplete 重复发奖励�?

      type_ = NpcDlgEnum::kQuestYes;

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
    RunScript(script_id_);

    ResetPos();

    return;
  }

  if (quest_id_.empty()) {
    return;
  }

  // 根据 type 决定后缀，统一拼接路径

  std::u16string suffix;

  const bool is_stop_page =
      type_ == NpcDlgEnum::kQuestStopItem ||
      type_ == NpcDlgEnum::kQuestStopMob ||
      type_ == NpcDlgEnum::kQuestStopLost ||
      type_ == NpcDlgEnum::kQuestStopNpc;

  if (is_stop_page && !stop_say_path_.empty()) {
    // CheckCompleteDemand 选出的专项失败对白（stop/level 等）。
    suffix = u"/" + stop_say_path_;
  } else {
    switch (type_) {
    case NpcDlgEnum::kQuestComplete:

    case NpcDlgEnum::kQuest:

      suffix = u"";

      break;

    case NpcDlgEnum::kQuestYes:

      suffix = u"/yes";

      break;

    case NpcDlgEnum::kQuestNo:

      suffix = u"/no";

      break;

    case NpcDlgEnum::kQuestStopItem:

      suffix = u"/item";

      break;

    case NpcDlgEnum::kQuestStopMob:

      suffix = u"/mob";

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
  }

  auto node = QuestGameInstance::LoadQuestNode(quest_id_);

  if (node) {
    node = node->find(u"Say/" + quest_index_ + suffix);
  }

  if (node != nullptr) {
    if (auto* page = node->get_child(std::to_string(index_));
        page != nullptr) {
      text_ = TextGameInstance::LoadRstr(page);
    }
  }

  ResetPos();

  time_ = Window::dt_now_;
}


void NpcDlgUiSystem::EventButtonPrev() {
  // 首页没有上一页：095 SayNormal 里 v4 < 0 直接结束对话，
  // 这里保持对话框不关闭、文本不变，避免取到空节点。
  if (index_ <= 0) {
    return;
  }

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

      ConfirmSelect();

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

// 095 CQuest::AskQuestion 的客户端实现：
//   Say/<idx>/0            问题文本（含 #L0#..#Ln# 选项）
//   Say/<idx>/stop/<n>/answer  正确选项（1-based，各组相同）
//   Say/<idx>/stop/<n>/<sel>   第 n 次答错、选中 sel 时的提示
// 答对进入 Say/<idx>/1 并结算，答错显示提示后可重新选择。
void NpcDlgUiSystem::ConfirmSelect() {
  auto* qnode = QuestGameInstance::LoadQuestNode(quest_id_);
  if (qnode == nullptr) {
    return;
  }
  auto* say = qnode->find(u"Say/" + quest_index_);
  if (say == nullptr) {
    return;
  }

  auto st = select_;
  if (select_mouse_ != -1) {
    st = select_mouse_;
  }

  // 正确选项：从 stop/<n>/answer 取第一组（1-based）。
  int answer = 0;
  auto* stop = say->get_child(u"stop");
  if (stop != nullptr) {
    for (auto [k, v] : *stop->get_children()) {
      if (v.empty() || v[0] == nullptr) {
        continue;
      }
      if (auto* a = v[0]->get_child(u"answer");
          a != nullptr && a->type == wz::Type::Int) {
        answer = static_cast<wz::Property<int>*>(a)->get();
        break;
      }
    }
  }

  if (answer > 0 && answer - 1 == st) {
    // 答对：显示 Say/<idx>/1（下一项对白）并进入奖励结算页。
    auto* next = say->get_child(u"1");
    type_ = NpcDlgEnum::kQuestComplete;
    if (next != nullptr) {
      text_ = TextGameInstance::LoadRstr(next);
    }
    time_ = Window::dt_now_;
    index_ = 0;
    max_index_ = 0;
    select_retry_ = false;
    select_attempt_ = 0;

    act_meso_ = QuestGameInstance::LoadQuestActMeso(quest_id_);
    act_item_ = QuestGameInstance::LoadQuestActItem(quest_id_);
    act_exp_ = QuestGameInstance::LoadQuestActExp(quest_id_);
    act_random_item_ = QuestGameInstance::LoadQuestActRandomItem(quest_id_);
    act_pop_ = QuestGameInstance::LoadQuestActPop(quest_id_);
    act_skill_ = QuestGameInstance::LoadQuestActSkill(quest_id_);
    return;
  }

  // 答错：Say/<idx>/stop/<attempt>/<sel>，没有对应提示时退回第 0 组。
  wz::Node* page = nullptr;
  if (stop != nullptr) {
    if (auto* g = stop->get_child(std::to_string(select_attempt_));
        g != nullptr) {
      page = g->get_child(std::to_string(st));
    }
    if (page == nullptr) {
      if (auto* g0 = stop->get_child(u"0"); g0 != nullptr) {
        page = g0->get_child(std::to_string(st));
      }
    }
  }

  if (page != nullptr) {
    type_ = NpcDlgEnum::kTalk;
    text_ = TextGameInstance::LoadRstr(page);
    time_ = Window::dt_now_;
    select_retry_ = true;
    ++select_attempt_;
    index_ = 0;
    max_index_ = 0;
    return;
  }

  // 没有提示对白：直接把问题页文本作为反馈，保持可重选。
  text_ = select_text_;
  time_ = Window::dt_now_;
  select_retry_ = true;
  ++select_attempt_;
  index_ = 0;
  max_index_ = 0;
}


void NpcDlgUiSystem::EventList() {
  if (type_ != NpcDlgEnum::kQuest) {
    return;
  }

  if (index_ != 0) {
    return;
  }

  if (type_ == NpcDlgEnum::kQuest && !script_id_.empty()) {
    RunScript(script_id_);

    return;
  }

  if (selected.empty()) {
    return;
  }

  // 点击任务列表条目：播放列表点击音

  AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");

  script_id_ = u"";

  stop_say_path_ = u"";

  select_retry_ = false;

  select_attempt_ = 0;

  quest_id_ = selected;


  auto progress = QuestGameInstance::LoadQuestProgress(selected);

  auto tmp = std::to_string(progress);

  quest_index_ = {tmp.begin(), tmp.end()};

  auto progress_complete = NpcGameInstance::LoadProgressCompleteQuest(npc_id_);


  wz::Node* node = nullptr;

  if (std::ranges::contains(progress_complete, quest_id_)) {
    // 待完�?

    node = QuestGameInstance::LoadQuestNode(quest_id_);

    if (auto n = node->find(u"Check/" + quest_index_ + u"/endscript");

        n != nullptr) {
      auto spt = static_cast<wz::Property<std::u16string>*>(n)->get();

      // 脚本未实现时退回普通的交任务流程（下面的对白与奖励结算照旧）。

      if (RunScript(spt)) {
        script_id_ = spt;

        return;
      }
    }

    if (node->find(u"Say/" + quest_index_ + u"/ask")) {
      if (auto* q_page = node->find(u"Say/" + quest_index_ + u"/0");
          q_page != nullptr) {
        text_ = TextGameInstance::LoadRstr(q_page);
      }

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

      // ask 问答状态：记录问题页文本，答错次数从 0 起。
      select_text_ = text_;
      select_attempt_ = 0;
      select_retry_ = false;

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

    act_pop_ = QuestGameInstance::LoadQuestActPop(quest_id_);

    act_skill_ = QuestGameInstance::LoadQuestActSkill(quest_id_);

    if (max_index_ == -1) {
      // 没有完成对白页时直接结算；有对白/奖励展示时不要跳过�?

      max_index_ = 0;

      index_ = max_index_;

      EventButtonOk();

      return;
    }


  } else if (QuestGameInstance::progress_quests_.contains(quest_id_)) {
    // 判断是否是进行中的任�?

    auto& q = QuestGameInstance::progress_quests_.at(quest_id_);

    // 二转试炼还没达标：直接送回试炼场继续，而不是只提示任务未完成。

    if (QuestGameInstance::EnterJobTestArena(quest_id_)) {
      Close();

      return;
    }

    // 095 CQuestMan::CheckCompleteDemand：按 level → item → mob 返回
    // 第一个不满足的原因，再用 Say/<idx>/stop/<reason> 作为提示对白。
    // 任务物品完全丢失时优先用 Say/<idx>/lost（095 OnCompleteQuestFailed）。
    auto reason = QuestGameInstance::CheckCompleteDemand(quest_id_);

    node = QuestGameInstance::LoadQuestNode(selected);

    auto* say = node != nullptr ? node->find(u"Say/" + quest_index_) : nullptr;

    stop_say_path_ = u"";

    if (!reason.empty()) {
      const auto pick = [&](const std::u16string& path) -> wz::Node* {
        return say != nullptr ? say->find(path) : nullptr;
      };
      if (pick(u"stop/" + reason) != nullptr) {
        stop_say_path_ = u"stop/" + reason;
      } else if (reason == u"item" && pick(u"lost") != nullptr) {
        stop_say_path_ = u"lost";
      } else if (pick(u"stop/default") != nullptr) {
        stop_say_path_ = u"stop/default";
      } else if (!q.item_bool && pick(u"stop/item") != nullptr) {
        stop_say_path_ = u"stop/item";
      } else if (!q.mob_bool && pick(u"stop/mob") != nullptr) {
        stop_say_path_ = u"stop/mob";
      } else if (pick(u"stop/npc") != nullptr) {
        stop_say_path_ = u"stop/npc";
      }
    }

    if (!stop_say_path_.empty()) {
      type_ = NpcDlgEnum::kQuestStopItem;

      // stop_say_path_ 可能是两级路径（stop/level），必须用 find。
      node = say != nullptr ? say->find(stop_say_path_) : nullptr;

      if (node == nullptr) {
        stop_say_path_ = u"";
      }
    }

    if (stop_say_path_.empty()) {
      if (!q.item_bool) {
        type_ = NpcDlgEnum::kQuestStopItem;

        node = QuestGameInstance::LoadQuestNode(selected);

        node = node->find(u"Say/" + quest_index_ + u"/stop/item");

      } else if (!q.mob_bool) {
        type_ = NpcDlgEnum::kQuestStopMob;

        node = QuestGameInstance::LoadQuestNode(selected);

        node = node->find(u"Say/" + quest_index_ + u"/stop/mob");

      } else {
        type_ = NpcDlgEnum::kQuestStopNpc;

        node = QuestGameInstance::LoadQuestNode(selected);

        node = node->find(u"Say/" + quest_index_ + u"/stop/npc");
      }
    }

    if (node) {
      max_index_ = node->children_count() - 1;
    }

  } else {
    type_ = NpcDlgEnum::kQuestAvaliable;

    node = QuestGameInstance::LoadQuestNode(selected);

    if (auto n = node->find(u"Check/0/startscript"); n != nullptr) {
      auto spt = static_cast<wz::Property<std::u16string>*>(n)->get();

      // 脚本未实现时退回普通的接任务对白。

      if (RunScript(spt)) {
        script_id_ = spt;

        return;
      }
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

    RunScript(script_id_);

    return;
  }

  // 接任务前先发�?Act/0 中对�?Check/1 的起始任务道具，

  // 例如 1001 的镜�?4031000�?

  auto start_items = QuestGameInstance::LoadQuestStartItem(quest_id_);

  auto back_data = PackageGameInstance::data_;

  for (auto [k, v] : start_items) {
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
  }


  // 接任务只改变任务状态，不发�?Act/1 的完成奖励�?

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


bool NpcDlgUiSystem::EventUI(SDL_Event* event) {
  return DispatchButtonClick(GetComponent(), true);
}


bool NpcDlgUiSystem::CursorIn() {
  auto [w, h] = LoadWh();

  auto& mouse = Window::mouse_pos_;

  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};

  return SDL_PointInRectFloat(&mouse, &pos_rect);
}


void NpcDlgUiSystem::EventMotionBtAudio(SDL_Event* event) {
  static int8_t bt_audio = -1;

  int8_t bt_in = static_cast<int8_t>(ButtonHoverIndex(GetComponent()));

  if (bt_in != -1 && bt_audio != bt_in) {
    AudioGameInstance::PlayNow(u"UI.img/BtMouseOver");
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
          if (type_ == NpcDlgEnum::kScript &&

              buttons_ == NpcDlgButton::kOk) {
            return false;
          }

          AudioGameInstance::PlayNow(u"UI.img/BtMouseClick");


          EventButtonClose();

          return false;

          break;
        }

        case SDL_SCANCODE_RETURN: {
          if (type_ == NpcDlgEnum::kScript) {
            if (buttons_ == NpcDlgButton::kOk) {
              EventButtonOk();

            } else if (buttons_ == NpcDlgButton::kYesNo) {
              EventButtonYes();

            } else if (index_ < max_index_) {
              EventButtonNext();

            } else {
              EventButtonClose();
            }

            return false;
          }

          if (index_ < max_index_) {
            EventButtonNext();

            return false;
          }

          if (index_ == max_index_ && cb_ == nullptr &&

              type_ == NpcDlgEnum::kQuestAvaliable) {
            EventButtonQuestYes();

            return false;
          }

          if (type_ == NpcDlgEnum::kSelect) {
            EventButtonNext();

            return false;
          }

          // ask 答错提示页：回车回到问题页重新选择。
          if (type_ == NpcDlgEnum::kTalk && select_retry_) {
            EventButtonOk();

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

          EventUI(event);

          if (type_ == NpcDlgEnum::kSelect && select_mouse_ >= 0) {
            // 鼠标点选同步到 select_，脚本回调里读到的才是鼠标选中的项

            select_ = select_mouse_;

            // 文本选择项确认音

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
    const auto current_quest = quest_id_;

    QuestGameInstance::CompleteQuest(current_quest);

    QuestAlarmUiSystem::CompleteQuest(current_quest);

    QuestUiSystem::CompleteQuest(current_quest);

    auto nq = static_cast<wz::Property<int>*>(n)->get();

    auto tmp = std::to_string(nq);

    selected = std::u16string{tmp.begin(), tmp.end()} + u".img";

    script_id_ = u"";

    index_ = 0;

    type_ = NpcDlgEnum::kQuest;

    EventList();

    return true;
  }

  return false;
}


}  // namespace sdlms
