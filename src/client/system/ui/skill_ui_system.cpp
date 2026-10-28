#include "src/client/system/ui/skill_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/game_instance/job_skill_game_instance.h"
#include "src/client/game_instance/skill_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/scroll_ui_system.h"
#include "src/client/system/ui/tooltip_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <wz/Node.h>
#include <wz/Property.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace sdlms {

SDL_FPoint SkillUiSystem::LoadWh() {
  return {197, 371};
}

bool SkillUiSystem::CursorIn() {
  auto [w, h] = LoadWh();
  auto& mouse = Window::mouse_pos_;
  SDL_FRect pos_rect{pos_.x, pos_.y, w, h};
  return SDL_PointInRectFloat(&mouse, &pos_rect);
}

void SkillUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void SkillUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::render_systems_, RenderInfo);
  std::erase(System::event_systems_, Event);

  EventDragEnd();
  EventVscrEnd();
}

void SkillUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);

    EventMotion(nullptr);
  }
}

void SkillUiSystem::EventDragStart(SDL_Event* Event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {Event->button.x, Event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - Event->button.x, pos_.y - Event->button.y};
  }
  return;
}

void SkillUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void SkillUiSystem::EventDragMove(SDL_Event* Event) {
  if (drag_.has_value()) {
    pos_ = {Event->motion.x + drag_->x, Event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto [w, h] = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - w);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - h);
  }
  return;
}

void SkillUiSystem::RenderBackgrnd() {
  static auto back_texture =
      WzResource::LoadTexture(WzResource::ui_->find(u"Skill.img/backgrnd"));
  SDL_FRect pos_rect = {
      pos_.x,
      pos_.y,
      (float)back_texture->w,
      (float)back_texture->h,
  };
  SDL_RenderTexture(Window::renderer_, back_texture, nullptr, &pos_rect);
}

void SkillUiSystem::RenderTab() {
  const static SDL_FPoint lt = {5, 24};
  const static SDL_FPoint rb = {186, 43};
  const static auto tab_node = WzResource::ui_->find(u"Skill.img/tab:grade");
  auto jobs =
      JobSkillGameInstance::LoadSkiTree(CharacterGameInstance::self_.job);
  const static std::array selected_texture = {
      WzResource::LoadTexture(tab_node->find(u"selected/0")),
      WzResource::LoadTexture(tab_node->find(u"selected/1")),
      WzResource::LoadTexture(tab_node->find(u"selected/2")),
      WzResource::LoadTexture(tab_node->find(u"selected/3")),
      WzResource::LoadTexture(tab_node->find(u"selected/4")),
  };
  const static std::array normal_texture = {
      WzResource::LoadTexture(tab_node->find(u"normal/0")),
      WzResource::LoadTexture(tab_node->find(u"normal/1")),
      WzResource::LoadTexture(tab_node->find(u"normal/2")),
      WzResource::LoadTexture(tab_node->find(u"normal/3")),
      WzResource::LoadTexture(tab_node->find(u"normal/4")),
  };
  for (uint8_t i = 0; i < jobs.size(); i++) {
    SDL_Texture* t = active_tab_ == i ? selected_texture[i] : normal_texture[i];
    SDL_FPoint tab_pos = {static_cast<float>(lt.x + i * 33), lt.y};
    SDL_FRect pos_rect{static_cast<float>(int(pos_.x + tab_pos.x)),
                       static_cast<float>(int(pos_.y + tab_pos.y)),
                       static_cast<float>(t->w), static_cast<float>(t->h)};
    SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);
  }
}

static std::u16string mouse_skill_id;

void SkillUiSystem::RenderSkillEntry() {
  mouse_skill_id = u"";
  auto entry =
      WzResource::LoadTexture(WzResource::ui_->find(u"Skill.img/entry/skill1"));

  const SDL_FPoint lt{8, 99};
  const SDL_FPoint rb{184, 334};
  const SDL_FPoint pos_icon{2, 2};
  const uint8_t max_scroll_num = 6;
  const auto l =
      (rb.y - lt.y - max_scroll_num * entry->h) / (max_scroll_num - 1);
  auto self_job = CharacterGameInstance::self_.job;
  auto ski_tree = JobSkillGameInstance::LoadSkiTree(self_job);
  JobType jt = ski_tree.at(active_tab_);

  auto& mouse_pos = Window::mouse_pos_;
  // 判断按钮是否被遮挡
  auto CursorIn = CursorGameInstance::cursor_ui;
  uint8_t i = 0;
  auto nodes = JobSkillGameInstance::LoadJobSkis(jt);
  for (auto it = nodes.begin() + page_; it != nodes.end(); ++it) {
    auto& k = it->first;
    auto& v = it->second;
    if (i >= max_scroll_num) {
      break;
    }
    auto skl_id = std::string{k.begin(), k.end()};
    auto skl_id2 = std::stoi(skl_id);

    SDL_Texture* ski_texture;
    auto ski_level = JobSkillGameInstance::LoadSkiLevel(k);
    SDL_FRect pos_rect = {
        pos_.x + lt.x + 2,
        pos_.y + lt.y + i * entry->h + l * i + 2,
        static_cast<float>(32),
        static_cast<float>(32),
    };
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      mouse_skill_id = k;
    }
    if (ski_level > 0) {
      if (SDL_PointInRectFloat(&mouse_pos, &pos_rect) && CursorIn == Render &&
          !CursorGameInstance::modal_overlay) {
        ski_texture =
            WzResource::LoadTexture(v[0]->get_child(u"iconMouseOver"));
      } else {
        ski_texture = WzResource::LoadTexture(v[0]->get_child(u"icon"));
      }
    } else {
      // disable
      ski_texture = WzResource::LoadTexture(v[0]->get_child(u"iconDisabled"));
      entry = WzResource::LoadTexture(
          WzResource::ui_->find(u"Skill.img/entry/skill0"));
    }
    // render backgrnd
    pos_rect = {
        pos_.x + lt.x,
        pos_.y + lt.y + i * entry->h + l * i,
        static_cast<float>(entry->w),
        static_cast<float>(entry->h),
    };
    SDL_RenderTexture(Window::renderer_, entry, nullptr, &pos_rect);

    pos_rect = {
        pos_.x + lt.x + 2,
        pos_.y + lt.y + i * entry->h + l * i + 2,
        static_cast<float>(ski_texture->w),
        static_cast<float>(ski_texture->h),
    };
    SDL_RenderTexture(Window::renderer_, ski_texture, nullptr, &pos_rect);
    // render ski_name
    auto ski_name = SkillGameInstance::LoadSkiName(k);
    Freetype::LoadColor(0, 0, 0, 255);
    Freetype::LoadAligned(true);
    Freetype::LoadSize(12);
    Freetype::DrawLine(ski_name.name, pos_.x + lt.x + 40, pos_rect.y - 3);
    auto ski_level2 = std::to_string(ski_level);
    auto ski_level3 = std::u16string{ski_level2.begin(), ski_level2.end()};
    Freetype::DrawLine(ski_level3, pos_.x + lt.x + 40, pos_rect.y + 15);

    Freetype::LoadAligned(false);
    i++;
  }
}

void SkillUiSystem::RenderScroll() {
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  auto self_job = CharacterGameInstance::self_.job;
  auto ski_tree = JobSkillGameInstance::LoadSkiTree(self_job);
  JobType jt = ski_tree.at(active_tab_);
  auto skill_node = JobSkillGameInstance::LoadJobSkis(jt);
  auto size = skill_node.size();
  auto CursorIn = CursorGameInstance::cursor_ui;
  bool top = (CursorIn == Render) && !CursorGameInstance::modal_overlay;
  ScrollUiSystem::RenderVscroll((int)pos_.x + lt.x, (int)pos_.y + lt.y, page_,
                                size, length, top, 6);
  return;
}

bool SkillUiSystem::RenderInfo() {
  if (!mouse_skill_id.empty() && !CursorGameInstance::modal_overlay &&
      CursorGameInstance::cursor_ui == Render) {
    auto& mouse_pos = Window::mouse_pos_;
    SDL_FPoint show_pos = {mouse_pos.x + 15, mouse_pos.y + 15};
    TooltipUiSystem::RenderSkill(mouse_skill_id, 1, show_pos.x, show_pos.y);
  }
  return true;
}

static std::u16string ski_button_id = u"";

void SkillUiSystem::RenderButton() {
  ski_button_id = u"";

  auto wh = LoadWh();
  auto self_job = CharacterGameInstance::self_.job;
  auto ski_tree = JobSkillGameInstance::LoadSkiTree(self_job);
  JobType jt = ski_tree.at(active_tab_);
  self_job = JobSkillGameInstance::LoadJobId(jt);
  auto skill_node = JobSkillGameInstance::LoadJobSkis(jt);

  const int max_scroll = 6;
  const int entry_h = 35;
  const float l = (334 - 99 - max_scroll * entry_h) / (max_scroll - 1);

  // 构建按钮列表
  std::vector<wz::Node*> nodes = {WzResource::ui_->find(u"Basic.img/BtClose")};
  std::vector<SDL_FRect> rects = {{wh.x - 20, 8, 12, 12}};

  const auto disable_ski = [](std::u16string id) {
    auto Active = active_tab_;
    const auto sps = JobSkillGameInstance::remain_point;
    if (sps.at(Active) == 0) {
      return true;
    }
    // 判断前置技能
    auto ski_node = SkillGameInstance::LoadSkiNode(id);
    if (auto req = ski_node->get_child(u"req")) {
      for (auto [k, v] : *req) {
        auto lv = static_cast<wz::Property<int>*>(v[0])->get();
        auto req_lv = JobSkillGameInstance::LoadSkiLevel(k);
        if (req_lv < lv) {
          return true;
        }
      }
    }
    auto ski_max_lvl = SkillGameInstance::LoadSkiMaxLvl(id);
    auto ski_lvl = JobSkillGameInstance::LoadSkiLevel(id);
    if (ski_lvl >= ski_max_lvl) {
      return true;
    }
    return false;
  };

  std::vector<bool> disable = {
      false,
  };

  for (int i = 0; i < max_scroll && i < skill_node.size(); ++i) {
    nodes.push_back(WzResource::ui_->find(u"Basic.img/BtUP"));
    rects.push_back({8 + 146, 99 + i * entry_h + l * i + 22, 12, 12});
    auto id = (skill_node.begin() + page_ + i)->first;
    disable.push_back(disable_ski(id));
  }

  // 渲染所有按钮
  bool mouse_down = Window::mouse_state_ & SDL_BUTTON_LMASK;
  bool cursor_on_ui = CursorGameInstance::cursor_ui == Render;
  bool modal_blocked = CursorGameInstance::modal_overlay;

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = {
        rects[i].x + (int)pos_.x,
        rects[i].y + (int)pos_.y,
        rects[i].w,
        rects[i].h,
    };

    std::u16string state = u"normal";
    bool mouse_in = SDL_PointInRectFloat(&Window::mouse_pos_, &rect);

    if (disable[i]) {
      state = u"disabled";
    } else if (cursor_on_ui && !modal_blocked && mouse_in) {
      state = mouse_down ? u"pressed" : u"mouseOver";
      if (i >= 1) {
        ski_button_id = (skill_node.begin() + page_ + (i - 1))->first;
      }
    }
    auto texture = WzResource::LoadTexture(nodes[i]->find((state + u"/0")));
    SDL_RenderTexture(Window::renderer_, texture, nullptr, &rect);
  }
}

void SkillUiSystem::RenderBook() {
  auto self_job = CharacterGameInstance::self_.job;
  auto ski_tree = JobSkillGameInstance::LoadSkiTree(self_job);
  JobType jt = ski_tree.at(active_tab_);
  auto job_str = JobSkillGameInstance::LoadJobId(jt);

  auto icon_node = WzResource::skill_->find(job_str + u".img/info/icon");
  auto t = WzResource::LoadTexture(icon_node);
  SDL_FRect pos_rect{
      pos_.x + 15,
      pos_.y + 57,
      static_cast<float>(t->w),
      static_cast<float>(t->h),
  };
  SDL_RenderTexture(Window::renderer_, t, nullptr, &pos_rect);

  auto book_node =
      WzResource::string_->find(u"Skill.img/" + job_str + u"/bookName");
  auto book_name = static_cast<wz::Property<std::u16string>*>(book_node)->get();
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);
  Freetype::LoadColor(255, 255, 255, 255);
  Freetype::DrawLine(book_name, pos_rect.x + 35, pos_rect.y + 6);
}

void SkillUiSystem::RenderPoint() {
  auto point = JobSkillGameInstance::remain_point[active_tab_];
  Freetype::LoadAligned(true);
  Freetype::LoadSize(12);
  Freetype::LoadColor(0, 0, 0, 255);
  auto str = std::to_string(point);
  std::u16string str2{str.begin(), str.end()};
  auto w = Freetype::LoadW(str2);
  Freetype::DrawLine(str2, pos_.x + 104 - w, pos_.y + 347);
}

bool SkillUiSystem::Render() {
  RenderBackgrnd();
  RenderSkillEntry();
  RenderButton();
  RenderScroll();
  RenderTab();
  RenderBook();
  RenderPoint();
  return true;
}

void SkillUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

void SkillUiSystem::EventButtonSkiUp() {
  if (ski_button_id == u"") {
    return;
  }
  auto ski_id =
      std::stoi(std::string(ski_button_id.begin(), ski_button_id.end()));
  JobSkillGameInstance::skill_point_[ski_id] += 1;
  JobSkillGameInstance::remain_point[active_tab_] -= 1;
  CharacterStatGameInstance::Update();
  return;
}

void SkillUiSystem::EventClose() {
  Close();
}

bool SkillUiSystem::EventButton(SDL_Event* Event) {
  std::vector<SDL_FRect> r;
  std::vector<void (*)()> fns;
  auto wh = LoadWh();
  r = {
      SDL_FRect{wh.x - 20, 7, 12, 12},  //
  };
  fns = {
      EventClose,
  };

  for (size_t i = 0; i < r.size(); ++i) {
    auto pos_rect = r[i];
    pos_rect.x += pos_.x;
    pos_rect.y += pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

bool SkillUiSystem::EventClickSki(SDL_Event* Event) {
  if (CursorGameInstance::cursor_hand_net_.has_value()) {
    return false;
  }
  if (!mouse_skill_id.empty()) {
    if (Event->button.clicks >= 1) {
      auto id = mouse_skill_id;
      if (!SkillGameInstance::LoadSkiActive(id)) {
        return false;
      }
      if (JobSkillGameInstance::LoadSkiLevel(id) == 0) {
        return false;
      }
      auto val = std::string{id.begin(), id.end()};
      auto sub_val = std::stoi(val);

      if (CursorGameInstance::cursor_hand_.has_value()) {
        if (CursorGameInstance::cursor_hand_->type ==
            CursorGameInstance::CursorHandType::kSkill) {
          if (CursorGameInstance::cursor_hand_->sub_val == sub_val) {
            // run_skill
            auto& c = CharacterGameInstance::self_;
            CharacterLogicSystem::RunSkill(c, id);
          }
        }
        CursorGameInstance::cursor_hand_ = std::nullopt;
        AudioGameInstance::LoadAudio(u"UI.img/DragEnd", 0);
        return false;
      } else {
        AudioGameInstance::LoadAudio(u"UI.img/DragStart", 0);
        CursorGameInstance::cursor_hand_ = {
            .type = CursorGameInstance::CursorHandType::kSkill,
            .val = active_tab_,
            .sub_val = static_cast<uint32_t>(sub_val),
        };
      }
    }
    return true;
  }
  return false;
}

bool SkillUiSystem::EventClickTab(SDL_Event* Event) {
  const static SDL_FPoint lt = {5, 24};
  const static SDL_FPoint rb = {186, 43};
  auto jobs =
      JobSkillGameInstance::LoadSkiTree(CharacterGameInstance::self_.job);
  auto& mouse_pos = Window::mouse_pos_;
  for (uint8_t i = 0; i < jobs.size(); i++) {
    SDL_FPoint tab_pos = {static_cast<float>(lt.x + i * 33), lt.y};
    SDL_FRect pos_rect{
        static_cast<float>(int(pos_.x + tab_pos.x)),
        static_cast<float>(int(pos_.y + tab_pos.y)),
        33,
        19,
    };
    if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
      AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
      active_tab_ = i;
      page_ = 0;
      return true;
    }
  }
  return false;
}

void SkillUiSystem::EventVscr(SDL_Event* Event) {
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  auto self_job = CharacterGameInstance::self_.job;
  auto ski_tree = JobSkillGameInstance::LoadSkiTree(self_job);
  JobType jt = ski_tree.at(active_tab_);
  // 根据active_tab获取技能组
  auto skill_node = JobSkillGameInstance::LoadJobSkis(jt);
  int size = skill_node.size() - 6;
  auto CursorIn = CursorGameInstance::cursor_ui;
  bool top = CursorIn == Render;
  size = std::max(0, size);
  auto val = ScrollUiSystem::ClickVscroll(
      (int)pos_.x + lt.x, (int)pos_.y + lt.y, page_, size, length, top);
  page_ = val;
}

void SkillUiSystem::EventMotion(SDL_Event* Event) {
  auto& sys = System::render_systems_;
  std::erase(sys, RenderInfo);
  auto it = std::ranges::find(sys, &CursorRenderSystem::Render);
  if (it != sys.end()) {
    sys.insert(it, RenderInfo);
  }
}

void SkillUiSystem::EventVscrStart(SDL_Event* Event) {
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  if (vscr_motion_ == false) {
    vscr_motion_ =
        ScrollUiSystem::ClickThumb(pos_.x + lt.x, pos_.y + lt.y, length);
  }
}

void SkillUiSystem::EventVscrEnd() {
  vscr_motion_ = {};
}

void SkillUiSystem::EventVscrMove(SDL_Event* Event) {
  auto mouse_state = Window::mouse_state_;
  const SDL_FPoint lt{174, 98};
  const uint32_t length = 236;
  if (vscr_motion_) {
    Event->button.x = pos_.x + lt.x;
    Event->button.y =
        std::clamp(Event->button.y, pos_.y + lt.y, pos_.y + lt.y + length);
    EventVscr(Event);
  }
}

void SkillUiSystem::EventWheel(SDL_Event* Event) {
  if (CursorGameInstance::cursor_ui != Render) {
    return;
  }
  auto dy = Event->wheel.integer_y;
  if (dy > 0) {
    // up
    if (page_ > 0) {
      page_ -= 1;
    }
  } else {
    // down
    auto self_job = CharacterGameInstance::self_.job;
    auto ski_tree = JobSkillGameInstance::LoadSkiTree(self_job);
    JobType jt = ski_tree.at(active_tab_);
    // 根据active_tab获取技能组
    auto skill_node = JobSkillGameInstance::LoadJobSkis(jt);
    int size = skill_node.size() - 6;
    size = std::max(0, size);
    if (page_ < size) {
      page_++;
    }
  }
}

void SkillUiSystem::EventMotionBtAudio(SDL_Event* Event) {
  if (CursorGameInstance::cursor_ui != Render) {
    return;
  }
  static std::u16string ski_button_i;
  if (!ski_button_id.empty() && ski_button_i.empty()) {
    AudioGameInstance::LoadAudio(u"UI.img/BtMouseOver", 0);
    ski_button_i = ski_button_id;
    return;
  }
  ski_button_i = ski_button_id;

  auto wh = LoadWh();
  std::vector<SDL_FRect> buttons_rect = {
      SDL_FRect{wh.x - 20, 7, 12, 12},  //
  };
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

bool SkillUiSystem::Event(SDL_Event* Event) {
  bool r = true;
  switch (Event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = Event->key.scancode;
      switch (scan_code) {
        case SDL_SCANCODE_ESCAPE: {
          EventClose();
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
      if (Event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui == Render) {
          EventTop();
          EventDragStart(Event);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (Event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui == Render) {
          EventButtonSkiUp();
          EventVscr(Event);
          EventClickSki(Event);
          EventClickTab(Event);
          r = EventButton(Event);
        }
        EventDragEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventMotionBtAudio(Event);
      EventMotion(Event);
      EventDragMove(Event);
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      EventWheel(Event);
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

}  // namespace sdlms
