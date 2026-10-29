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
//
// Each UI system follows the same immediate-mode lifecycle:
//
//   GetComponent()  rebuilds the component list for the current frame state.
//   RenderUI()      draws the list in order.
//   EventUI()       clicks the list; returns true when a component consumed
//                   the click, so Event() reports false to stop event dispatch.
//
// Lists are rebuilt per call, so screen positions always match the cursor.
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

// Shared storage and drawing for a UI's component list.
//
// A UI system derives from this, implements GetComponent() (rebuild the list)
// and EventUI() (click the list); the component storage and RenderUI() come
// from here.
template <class Derived>
class UiComponentHost {
 protected:
  static inline UIComponents components_;

 public:
  static void RenderUI() { RenderComponents(Derived::GetComponent()); }
};

}  // namespace sdlms
