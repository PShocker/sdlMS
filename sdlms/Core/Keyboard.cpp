#include "Keyboard.h"

void Keyboard::init()
{
    action[UP] = SDL_SCANCODE_UP;
    action[DOWN] = SDL_SCANCODE_DOWN;
    action[LEFT] = SDL_SCANCODE_LEFT;
    action[RIGHT] = SDL_SCANCODE_RIGHT;
    action[JUMP] = SDL_SCANCODE_LALT;
    action[SIT] = SDL_SCANCODE_X;
    action[PICK] = SDL_SCANCODE_Z;
    action[ATTACK] = SDL_SCANCODE_LCTRL;

    ui[SDL_SCANCODE_W] = UIIndex::UI_WorldMap;
    ui[SDL_SCANCODE_I] = UIIndex::UI_UIItem;

    skill[SDL_SCANCODE_A] = 14101006;
    skill[SDL_SCANCODE_SPACE] = 2201002;
    skill[SDL_SCANCODE_F] = 2221006;
    skill[SDL_SCANCODE_Y] = 1111008;
    skill[SDL_SCANCODE_D] = 1101004;
    skill[SDL_SCANCODE_S] = 1311006;
    skill[SDL_SCANCODE_G] = 4111005;
    skill[SDL_SCANCODE_H] = 4111002;
    skill[SDL_SCANCODE_C] = 4001003;
    skill[SDL_SCANCODE_V] = 3001005;
    skill[SDL_SCANCODE_B] = 4211006;
    skill[SDL_SCANCODE_J] = 1311001;
    skill[SDL_SCANCODE_K] = 1121006;
    skill[SDL_SCANCODE_R] = 4211002;
    skill[SDL_SCANCODE_T] = 4201004;
    skill[SDL_SCANCODE_U] = 1121008;
    skill[SDL_SCANCODE_N] = 4121007;
    skill[SDL_SCANCODE_Q] = 2201004;
    skill[SDL_SCANCODE_E] = 4101004;
    skill[SDL_SCANCODE_M] = 1101007;
    skill[SDL_SCANCODE_1] = 2221005;
    skill[SDL_SCANCODE_2] = 2121005;
    skill[SDL_SCANCODE_3] = 3121006;
    skill[SDL_SCANCODE_4] = 3101005;
    skill[SDL_SCANCODE_5] = 2001002;
    skill[SDL_SCANCODE_6] = 21100005;
    skill[SDL_SCANCODE_7] = 3221005;
    skill[SDL_SCANCODE_O] = 4201005;
    skill[SDL_SCANCODE_P] = 3111003;
    skill[SDL_SCANCODE_L] = 2101004;
}