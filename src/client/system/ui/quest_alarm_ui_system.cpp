#include "quest_alarm_ui_system.h"
#include "src/common/wz/wz_resource.h"

void quest_alarm_ui_system::render_backgrnd() {
  static auto backgrndmax = wz_resource::load_texture(
      wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndmax"));
  static auto backgrndcenter = wz_resource::load_texture(
      wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndcenter"));
  static auto backgrndbottom = wz_resource::load_texture(
      wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndbottom"));
  static auto backgrndmin = wz_resource::load_texture(
      wz_resource::ui->find(u"Quest.img/QuestAlarm/backgrndmin"));
}

void quest_alarm_ui_system::render_quests() {}

bool quest_alarm_ui_system::render() {
  render_backgrnd();
  render_quests();
  return true;
}