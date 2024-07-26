#include "S_Cursor.h"
#include <ranges>
#include <algorithm>
#include <SDL2/SDL.h>
#include "Components/C_Cursor.h"
#include "Core/Input.h"

void S_Cursor::run(World &world)
{
    if (world.components_exist_of_type<C_Cursor>())
    {
        for (auto &[index, pla] : world.get_components<C_Cursor>())
        {
            if (Input::is_mouse_button_pressed(Input::MouseButton::ButtonLeft))
            {
                switch_act(pla, C_Cursor::State::CLICKING);
            }
            else if (Input::is_mouse_button_released(Input::MouseButton::ButtonLeft))
            {
                switch_act(pla, C_Cursor::State::IDLE);
            }

            update_Transform(pla->get_owner_component<Transform>());
        }
    }
}

void S_Cursor::switch_act(C_Cursor *cCursor, C_Cursor::State action)
{
    if (action != cCursor->act)
    {
        // if (cCursor->aspr_map.contains(action))
        {
            auto aspr = cCursor->aspr_map[action];
            aspr->set_anim_index(0);
            aspr->set_anim_time(0);
            cCursor->act = action;
            cCursor->get_owner()->add_component(aspr);
        }
    }
}

void S_Cursor::update_Transform(Transform *t)
{
    int x, y;
    Uint32 mouse_state = SDL_GetMouseState(&x, &y);
    t->set_position(SDL_FPoint(x, y));
}