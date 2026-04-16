#pragma once

#include "ui_base.h"
#include "ui_button.h"
class ui_skill_slot : public ui_button {
public:
  void hover() override;
  void hover_leave() override;
};