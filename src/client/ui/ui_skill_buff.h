#pragma once

#include "ui_button.h"
class ui_skill_buff : public ui_button {
public:
  uint32_t index;

  void render() override;
};