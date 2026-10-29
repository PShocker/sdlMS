#include "src/client/system/ui/character_stat_ui_system.h"

#include "src/client/game_instance/audio_game_instance.h"
#include "src/client/game_instance/camera_game_instance.h"
#include "src/client/game_instance/character_game_instance.h"
#include "src/client/game_instance/character_stat_game_instance.h"
#include "src/client/game_instance/cursor_game_instance.h"
#include "src/client/system/logic/character_logic_system.h"
#include "src/client/system/render/cursor_render_system.h"
#include "src/client/system/system.h"
#include "src/client/system/ui/button.h"
#include "src/client/system/ui/notice_ui_system.h"
#include "src/client/window/window.h"
#include "src/common/freetype/freetype.h"
#include "src/common/wz/wz_resource.h"

#include <SDL3/SDL_rect.h>

#include <algorithm>
#include <string>

namespace sdlms {

SDL_FPoint CharacterStatUiSystem::LoadWh() {
  return {185, 281};
}

static const SDL_FRect detail_rect = {185, 58, 187, 223};

void CharacterStatUiSystem::RenderBackgrnd() {
  static auto backgrnd = WzResource::LoadTexture(
      WzResource::ui_->find(u"CharacterStat.img/Main/backgrnd"));
  SDL_FRect pos_rect{pos_.x, pos_.y, static_cast<float>(backgrnd->w),
                     static_cast<float>(backgrnd->h)};
  SDL_RenderTexture(Window::renderer_, backgrnd, nullptr, &pos_rect);
}

void CharacterStatUiSystem::RenderText() {
  SDL_FPoint p{pos_.x + 60, pos_.y + 32};
  // name
  const auto& self = CharacterGameInstance::self_;
  auto& self_name = self.name;
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  Freetype::DrawLine(self_name, p.x, p.y);
  // job
  auto self_job = self.job;
  p = {pos_.x + 61, pos_.y + 55};
  static auto job_node = WzResource::ui_->find(u"CharacterStat.img/Main/Job");
  auto job_texture = WzResource::LoadTexture(
      job_node->find(u"main/" + std::u16string{self_job[0]}));
  SDL_FRect pos_rect{
      static_cast<float>((int)p.x),
      static_cast<float>((int)p.y),
      static_cast<float>(job_texture->w),
      static_cast<float>(job_texture->h),
  };
  SDL_RenderTexture(Window::renderer_, job_texture, nullptr, &pos_rect);

  // sub job
  auto job_sub_node = job_node->find(u"sub/" + std::u16string{self_job[0]});
  if (self_job == u"000") {
    self_job = u"0";
  }
  auto sub_job_texture =
      WzResource::LoadTexture(job_sub_node->get_child(self_job));
  pos_rect = {
      static_cast<float>((int)pos_.x + 61),
      static_cast<float>((int)pos_.y + 67),
      static_cast<float>(sub_job_texture->w),
      static_cast<float>(sub_job_texture->h),
  };
  SDL_RenderTexture(Window::renderer_, sub_job_texture, nullptr, &pos_rect);

  // level
  p = {pos_.x + 60, pos_.y + 79};
  auto level = self.level;
  auto level1 = std::to_string(level);
  auto level2 = std::u16string{level1.begin(), level1.end()};
  Freetype::DrawLine(level2, p.x, p.y);

  // hp
  p = {pos_.x + 60, pos_.y + 97};
  auto hp = CharacterStatGameInstance::GetHp();
  auto hp1 = std::to_string(hp);
  auto hp2 = std::u16string{hp1.begin(), hp1.end()};
  Freetype::DrawLine(hp2, p.x, p.y);

  // mp
  p = {pos_.x + 60, pos_.y + 115};
  auto mp = CharacterStatGameInstance::GetMp();
  auto mp1 = std::to_string(mp);
  auto mp2 = std::u16string{mp1.begin(), mp1.end()};
  Freetype::DrawLine(mp2, p.x, p.y);

  // exp
  p = {pos_.x + 60, pos_.y + 133};
  auto exp = CharacterStatGameInstance::GetExp();
  auto exp1 = std::to_string(exp);
  auto exp2 = std::u16string{exp1.begin(), exp1.end()};
  Freetype::DrawLine(exp2, p.x, p.y);

  // fame
  p = {pos_.x + 60, pos_.y + 151};
  auto fame = self.fame;
  auto fame1 = std::to_string(fame);
  auto fame2 = std::u16string{fame1.begin(), fame1.end()};
  Freetype::DrawLine(fame2, p.x, p.y);

  // str
  p = {pos_.x + 60, pos_.y + 175};
  auto str = CharacterStatGameInstance::GetStrAp();
  auto str1 = std::to_string(str);
  auto str2 = std::u16string{str1.begin(), str1.end()};
  Freetype::DrawLine(str2, p.x, p.y);

  // dex
  p = {pos_.x + 60, pos_.y + 193};
  auto dex = CharacterStatGameInstance::GetDexAp();
  auto dex1 = std::to_string(dex);
  auto dex2 = std::u16string{dex1.begin(), dex1.end()};
  Freetype::DrawLine(dex2, p.x, p.y);

  // int
  p = {pos_.x + 60, pos_.y + 211};
  auto in = CharacterStatGameInstance::GetIntAp();
  auto in1 = std::to_string(in);
  auto in2 = std::u16string{in1.begin(), in1.end()};
  Freetype::DrawLine(in2, p.x, p.y);

  // luk
  p = {pos_.x + 60, pos_.y + 229};
  auto luk = CharacterStatGameInstance::GetLukAp();
  auto luk1 = std::to_string(luk);
  auto luk2 = std::u16string{luk1.begin(), luk1.end()};
  Freetype::DrawLine(luk2, p.x, p.y);
}

void CharacterStatUiSystem::RenderRemainAp() {
  auto ap = CharacterStatGameInstance::GetRemainAp();
  auto s = std::to_string(ap);
  auto s2 = std::u16string{s.begin(), s.end()};
  Freetype::LoadSize(12);
  Freetype::LoadAligned(true);
  Freetype::LoadColor(0, 0, 0, 255);
  Freetype::DrawLine(s2, pos_.x + 90, pos_.y + 258);
}

void CharacterStatUiSystem::RenderButton() {
  // 构建按钮列表
  std::vector<wz::Node*> nodes = {
      WzResource::ui_->find(u"Basic.img/BtClose"),
      WzResource::ui_->find(u"CharacterStat.img/Main/button:detail"),
      WzResource::ui_->find(u"Basic.img/BtUP"),  // hp
      WzResource::ui_->find(u"CharacterStat.img/Main/button:hpupall"),
      WzResource::ui_->find(u"Basic.img/BtUP"),  // mp
      WzResource::ui_->find(u"CharacterStat.img/Main/button:hpupall"),
      WzResource::ui_->find(u"Basic.img/BtUP"),  // str
      WzResource::ui_->find(u"CharacterStat.img/Main/button:hpupall"),
      WzResource::ui_->find(u"Basic.img/BtUP"),  // dex
      WzResource::ui_->find(u"CharacterStat.img/Main/button:hpupall"),
      WzResource::ui_->find(u"Basic.img/BtUP"),  // int
      WzResource::ui_->find(u"CharacterStat.img/Main/button:hpupall"),
      WzResource::ui_->find(u"Basic.img/BtUP"),  // luk
      WzResource::ui_->find(u"CharacterStat.img/Main/button:hpupall"),
  };
  std::vector<SDL_FRect> rects = {
      {166, 6, 12, 12},    //
      {122, 256, 47, 18},  //
      {148, 99, 12, 12},   //
      {161, 99, 12, 12},   //
      {148, 117, 12, 12},  //
      {161, 117, 12, 12},  //
      {148, 177, 12, 12},  // str
      {161, 177, 12, 12},  // strall
      {148, 195, 12, 12},  // dex
      {161, 195, 12, 12},  // dexall
      {148, 213, 12, 12},  // dex
      {161, 213, 12, 12},  // dexall
      {148, 231, 12, 12},  // dex
      {161, 231, 12, 12},  // dexall
  };
  std::vector<bool> disable = {};
  auto ap = CharacterStatGameInstance::GetRemainAp();
  if (ap) {
    disable = {
        false, false, false, false, false, false, false,
        false, false, false, false, false, false, false,
    };
  } else {
    disable = {
        false, true, true, true, true, true, true,
        true,  true, true, true, true, true, true,
    };
  }

  if (detail_) {
    nodes.push_back(WzResource::ui_->find(u"Basic.img/BtHide"));
    rects.push_back({164 + detail_rect.x, 203 + detail_rect.y, 13, 12});
    disable.push_back(false);
  }
  // 渲染所有按钮

  for (size_t i = 0; i < nodes.size(); ++i) {
    SDL_FRect rect = rects[i];
    rect.x += (int)pos_.x;
    rect.y += (int)pos_.y;
    Button button{.node = nodes[i],
                  .rect = rect,
                  .enable = !disable[i],
                  .focused = CursorGameInstance::cursor_ui_ == Render &&
                             !CursorGameInstance::modal_overlay_};
    button.Render();
  }
}

void CharacterStatUiSystem::RenderDetail() {
  static auto backgrnd_detail = WzResource::LoadTexture(
      WzResource::ui_->find(u"CharacterStat.img/Detail/backgrnd"));
  SDL_FRect pos_rect = {pos_.x + detail_rect.x, pos_.y + detail_rect.y,
                        static_cast<float>(backgrnd_detail->w),
                        static_cast<float>(backgrnd_detail->h)};
  SDL_RenderTexture(Window::renderer_, backgrnd_detail, nullptr, &pos_rect);

  auto dx = detail_rect.x + pos_.x;
  auto dy = detail_rect.y + pos_.y;
  // atk
  SDL_FPoint p = {dx + 78, dy + 10};
  auto min_atk = CharacterStatGameInstance::GetMinAtk();
  auto max_atk = CharacterStatGameInstance::GetMaxAtk();
  auto s = std::to_string(min_atk) + " - " + std::to_string(max_atk);
  auto s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
  // pdd
  p = {dx + 78, dy + 28};
  auto pdd = CharacterStatGameInstance::GetDef();
  s = std::to_string(pdd);
  s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
  // mad
  p = {dx + 78, dy + 46};
  auto mad = CharacterStatGameInstance::GetMagic();
  s = std::to_string(mad);
  s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
  // mdd
  p = {dx + 78, dy + 62};
  auto mdd = CharacterStatGameInstance::GetMdef();
  s = std::to_string(mdd);
  s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
  // acc
  p = {dx + 78, dy + 82};
  auto acc = CharacterStatGameInstance::GetAcc();
  s = std::to_string(acc);
  s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
  // eva
  p = {dx + 78, dy + 100};
  auto avoid = CharacterStatGameInstance::GetAvd();
  s = std::to_string(avoid);
  s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
  // crtr
  p = {dx + 78, dy + 118};
  auto crit_rate = CharacterStatGameInstance::GetCritRate();
  s = std::to_string(crit_rate);
  s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
  // crtd
  p = {dx + 78, dy + 136};
  auto crit_damage = CharacterStatGameInstance::GetCritDamage();
  s = std::to_string(crit_damage);
  s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
  // hspd
  p = {dx + 78, dy + 154};
  auto hspd = CharacterLogicSystem::self_hspeed_max_;
  s = std::to_string((int)hspd);
  s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
  // vspd
  p = {dx + 78, dy + 172};
  auto vspd = -CharacterLogicSystem::self_vspeed_min_ * 0.1;
  s = std::to_string((int)vspd);
  s2 = std::u16string{s.begin(), s.end()};
  Freetype::DrawLine(s2, p.x, p.y);
}

bool CharacterStatUiSystem::Render() {
  RenderBackgrnd();
  RenderText();
  RenderRemainAp();
  if (detail_) {
    RenderDetail();
  }
  RenderButton();
  return true;
}

void CharacterStatUiSystem::Open() {
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    auto wh = LoadWh();
    auto& camera = CameraGameInstance::camera_;
    pos_.x = (camera.w - wh.x) / 2;
    pos_.y = (camera.h - wh.y) / 2;

    detail_ = true;

    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void CharacterStatUiSystem::Close() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);

  EventDragEnd();
}

void CharacterStatUiSystem::Toggle() {
  AudioGameInstance::LoadAudio(u"UI.img/BtMouseClick", 0);
  auto fn = &Render;
  if (std::ranges::contains(System::render_systems_, fn)) {
    Close();
  } else {
    Open();
  }
}

void CharacterStatUiSystem::EventButtonDetailShow() {
  detail_ = true;
}

void CharacterStatUiSystem::EventButtonDetailHide() {
  detail_ = false;
}

void CharacterStatUiSystem::EventButtonHpInc() {
  auto hp_ap = CharacterStatGameInstance::GetHpAp();
  CharacterStatGameInstance::SetHpAp(hp_ap + 1);
  auto r_ap = CharacterStatGameInstance::GetRemainAp();
  CharacterStatGameInstance::SetRemainAp(r_ap - 1);
}

void CharacterStatUiSystem::EventButtonHpIncMax() {
  NoticeUiSystem::data_ = 0;
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kApInc;
  NoticeUiSystem::Open();
}

void CharacterStatUiSystem::EventButtonMpInc() {
  auto mp_ap = CharacterStatGameInstance::GetMpAp();
  CharacterStatGameInstance::SetMpAp(mp_ap + 1);
  auto r_ap = CharacterStatGameInstance::GetRemainAp();
  CharacterStatGameInstance::SetRemainAp(r_ap - 1);
}

void CharacterStatUiSystem::EventButtonMpIncMax() {
  NoticeUiSystem::data_ = 1;
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kApInc;
  NoticeUiSystem::Open();
}

void CharacterStatUiSystem::EventButtonStrInc() {
  auto str_ap = CharacterStatGameInstance::GetStrAp();
  CharacterStatGameInstance::SetStrAp(str_ap + 1);
  auto r_ap = CharacterStatGameInstance::GetRemainAp();
  CharacterStatGameInstance::SetRemainAp(r_ap - 1);
}

void CharacterStatUiSystem::EventButtonStrIncMax() {
  NoticeUiSystem::data_ = 2;
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kApInc;
  NoticeUiSystem::Open();
}

void CharacterStatUiSystem::EventButtonDexInc() {
  auto dex_ap = CharacterStatGameInstance::GetDexAp();
  CharacterStatGameInstance::SetDexAp(dex_ap + 1);
  auto r_ap = CharacterStatGameInstance::GetRemainAp();
  CharacterStatGameInstance::SetRemainAp(r_ap - 1);
}

void CharacterStatUiSystem::EventButtonDexIncMax() {
  NoticeUiSystem::data_ = 3;
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kApInc;
  NoticeUiSystem::Open();
}

void CharacterStatUiSystem::EventButtonIntInc() {
  auto int_ap = CharacterStatGameInstance::GetIntAp();
  CharacterStatGameInstance::SetIntAp(int_ap + 1);
  auto r_ap = CharacterStatGameInstance::GetRemainAp();
  CharacterStatGameInstance::SetRemainAp(r_ap - 1);
}

void CharacterStatUiSystem::EventButtonIntIncMax() {
  NoticeUiSystem::data_ = 4;
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kApInc;
  NoticeUiSystem::Open();
}

void CharacterStatUiSystem::EventButtonLukInc() {
  auto luk_ap = CharacterStatGameInstance::GetLukAp();
  CharacterStatGameInstance::SetLukAp(luk_ap + 1);
  auto r_ap = CharacterStatGameInstance::GetRemainAp();
  CharacterStatGameInstance::SetRemainAp(r_ap - 1);
}

void CharacterStatUiSystem::EventButtonLukIncMax() {
  NoticeUiSystem::data_ = 5;
  NoticeUiSystem::type_ = NoticeUiSystem::NoticeEnum::kApInc;
  NoticeUiSystem::Open();
}

bool CharacterStatUiSystem::EventButton(SDL_Event* event) {
  std::vector<void (*)()> fns;
  auto wh = LoadWh();
  std::vector<SDL_FRect> buttons_rect = {
      {166, 6, 12, 12},    // close
      {122, 256, 47, 18},  // detail
      {148, 99, 12, 12},   // hp
      {161, 99, 12, 12},   // hpm
      {148, 117, 12, 12},  // mp
      {161, 117, 12, 12},  // mpm
      {148, 177, 12, 12},  // str
      {161, 177, 12, 12},  // strall
      {148, 195, 12, 12},  // dex
      {161, 195, 12, 12},  // dexall
      {148, 213, 12, 12},  // dex
      {161, 213, 12, 12},  // dexall
      {148, 231, 12, 12},  // dex
      {161, 231, 12, 12},  // dexall
  };
  fns = {
      Close,
      EventButtonDetailShow,
      EventButtonHpInc,
      EventButtonHpIncMax,
      EventButtonMpInc,
      EventButtonMpIncMax,
      EventButtonStrInc,
      EventButtonStrIncMax,
      EventButtonDexInc,
      EventButtonDexIncMax,
      EventButtonIntInc,
      EventButtonIntIncMax,
      EventButtonLukInc,
      EventButtonLukIncMax,
  };

  if (detail_) {
    buttons_rect.push_back({164 + detail_rect.x, 203 + detail_rect.y, 13, 12});
    fns.push_back(EventButtonDetailHide);
  }

  for (size_t i = 0; i < buttons_rect.size(); ++i) {
    auto pos_rect = buttons_rect[i];
    pos_rect.x += (int)pos_.x;
    pos_rect.y += (int)pos_.y;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      fns[i]();
      return true;
    }
  }

  return false;
}

bool CharacterStatUiSystem::Event(SDL_Event* event) {
  bool r = true;
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      auto scan_code = event->key.scancode;
      switch (scan_code) {
        case SDL_SCANCODE_ESCAPE: {
          Close();
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
          EventTop();
          EventDragStart(event);
          r = false;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (CursorGameInstance::cursor_ui_ == Render) {
          r = !EventButton(event);
        }
        EventDragEnd();
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      EventDragMove(event);
      break;
    }
    default: {
      break;
    }
  }

  return r;
}

void CharacterStatUiSystem::EventTop() {
  std::erase(System::render_systems_, Render);
  std::erase(System::event_systems_, Event);
  auto it =
      std::ranges::find(System::render_systems_, &CursorRenderSystem::Render);
  if (it != System::render_systems_.end()) {
    System::render_systems_.insert(it, Render);
    System::event_systems_.insert(System::event_systems_.begin(), Event);
  }
}

void CharacterStatUiSystem::EventDragStart(SDL_Event* event) {
  auto wh = LoadWh();
  SDL_FRect pos_rect = {pos_.x, pos_.y, wh.x, 20};
  SDL_FPoint mouse_pos = {event->button.x, event->button.y};
  if (SDL_PointInRectFloat(&mouse_pos, &pos_rect)) {
    drag_ = {pos_.x - event->button.x, pos_.y - event->button.y};
  }
  return;
}

void CharacterStatUiSystem::EventDragEnd() {
  drag_ = std::nullopt;
  return;
}

void CharacterStatUiSystem::EventDragMove(SDL_Event* event) {
  if (drag_.has_value()) {
    pos_ = {event->motion.x + drag_->x, event->motion.y + drag_->y};
    auto& camera = CameraGameInstance::camera_;
    auto wh = LoadWh();
    pos_.x = std::clamp(pos_.x, (float)0, camera.w - wh.x);
    pos_.y = std::clamp(pos_.y, (float)0, camera.h - wh.y);
  }
  return;
}

bool CharacterStatUiSystem::CursorIn() {
  auto wh = LoadWh();
  SDL_FRect pos_rect{pos_.x, pos_.y, wh.x, wh.y};
  if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
    return true;
  }
  if (detail_) {
    pos_rect.x = pos_.x + detail_rect.x;
    pos_rect.y = pos_.y + detail_rect.y;
    pos_rect.w = detail_rect.w;
    pos_rect.h = detail_rect.h;
    if (SDL_PointInRectFloat(&Window::mouse_pos_, &pos_rect)) {
      return true;
    }
  }
  return false;
}

}  // namespace sdlms
