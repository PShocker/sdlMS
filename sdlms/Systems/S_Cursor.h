#pragma once
#include "Systems/System.h"
#include "Components/AnimatedSprite.h"
#include "Components/Transform.h"
#include "Components/HVMove.h"
#include "Components/Avatar.h"

#include "Entities/Portal.h"

#include "S_Cursor.h"
#include <ranges>
#include <algorithm>
#include <SDL2/SDL.h>
#include "Components/C_Cursor.h"
#include "Core/Input.h"

class S_Cursor : public System
{
public:
	void run(World &world) override;

private:
	bool update_animated_sprite(AnimatedSprite *aspr, int delta_time, World &world);
	void update_avatar(Avatar *hvm, int delta_time, World &world);
	void switch_act(C_Cursor* cCursor, C_Cursor::State action);
	void update_Transform(Transform* cCursor);
};
