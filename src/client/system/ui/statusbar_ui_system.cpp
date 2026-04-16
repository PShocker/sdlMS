#include "statusbar_ui_system.h"

void statusbar_ui_system::load_button(){
    
}

statusbar_ui_system::statusbar_ui_system() {
  ui.clear();
  load_button();
  load_hp_bar();
  load_mp_bar();
  load_exp_bar();
}