#ifndef HEADER_SDLMS_INPUT
#define HEADER_SDLMS_INPUT

#include <SDL2/SDL.h>
#include <map>
#include "util/currenton.hpp"

class Input : public Currenton<Input>
{
public:
	void beginNewFrame();
	void keyUpEvent(const SDL_Event &event);
	void keyDownEvent(const SDL_Event &event);

	bool wasKeyPressed(SDL_Scancode key);
	bool wasKeyReleased(SDL_Scancode key);
	bool isKeyHeld(SDL_Scancode key);

private:
	std::map<SDL_Scancode, bool> _heldKeys;
	std::map<SDL_Scancode, bool> _pressedKeys;
	std::map<SDL_Scancode, bool> _releasedKeys;
};

#endif
