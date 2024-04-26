#pragma once
#include "Systems/System.h"
#include "Components/Sound.h"

class SoundSystem : public System
{
public:
	void run(World &world) override;

private:
	static int play_sound(void *sou);
};
