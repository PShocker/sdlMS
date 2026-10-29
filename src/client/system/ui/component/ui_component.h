#pragma once

#include "src/client/system/ui/component/button.h"
#include "src/client/system/ui/component/check_box.h"
#include "src/client/system/ui/component/combo_box.h"
#include "src/client/system/ui/component/scroll.h"
#include "src/client/system/ui/component/slider.h"
#include "src/client/system/ui/component/tab.h"
#include "src/client/system/ui/component/text_input.h"

#include <variant>
#include <vector>

namespace sdlms {

// One element of a UI's component list. The owning UI rebuilds the list in
// GetComponent() and RenderUI() draws every element in list order.
using UIComponent =
    std::variant<Button, CheckBox, ComboBox, Scroll, Slider, Tab, TextInput>;

using UIComponents = std::vector<UIComponent>;

// Draws every component in list order.
void RenderComponents(UIComponents& components);

// Fires the callback of the first enabled Button component under the cursor.
//
// Returns true when a click was consumed. `play_sound` plays the standard
// button click sound, mirroring the behaviour of each UI before the
// components were introduced.
bool DispatchButtonClick(UIComponents& components, bool play_sound);

// Returns the component index of the first Button component under the
// cursor, or -1.
int ButtonHoverIndex(UIComponents& components);

}  // namespace sdlms
