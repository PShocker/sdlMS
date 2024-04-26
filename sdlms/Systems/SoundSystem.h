#pragma once
#include "Systems/System.h"
#include "Components/Sound.h"

class SoundSystem : public System
{
public:
	void run(World &world) override;

private:
	void play_sound(Sound *sou, World &world);
};
