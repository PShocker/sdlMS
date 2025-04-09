#pragma once
#include "Component.h"
#include "wz/Property.hpp"
#include "Core/World.h"
#include "Components/AnimatedSprite.h"

class C_Cursor : public Component
{
    public:
enum State
		{
			IDLE,
			CANCLICK,
			GAME,
			HOUSE,
			CANCLICK2,
			CANGRAB,
			GIFT,
			VSCROLL,
			HSCROLL,
			VSCROLLIDLE,
			HSCROLLIDLE,
			GRABBING,
			CLICKING,
			RCLICK,
			LEAF = 18,
			CHATBARVDRAG = 67,
			CHATBARHDRAG,
			CHATBARBLTRDRAG,
			CHATBARMOVE = 72,
			CHATBARBRTLDRAG,
			LENGTH
		};

    C_Cursor(wz::Node *node, int id, int rx0, int rx1, World *world);
    C_Cursor(World *world);
    ~C_Cursor();

    void switch_act(State s);
    void stop_animate();
    void start_animate();

public:
    std::unordered_map<std::u16string, std::u16string> str_map;
    State act = State::IDLE;
    std::unordered_map<State, AnimatedSprite*> aspr_map;
    std::unordered_map<std::u16string, AnimatedSprite*> aspr_map1;
    //State state = State::IDLE;
};