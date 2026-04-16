#pragma once

#include "ui_base.h"
class ui_statusbar_mp : public ui_base {
public:
  uint32_t prev_mp;
  float prev_percent;
  bool alert;
};