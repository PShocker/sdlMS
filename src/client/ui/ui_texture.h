#pragma once

#include "ui_base.h"
#include <string>
class ui_texture : public ui_base {
public:
  std::u16string path;

  void render() override;
};