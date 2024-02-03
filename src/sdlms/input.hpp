#ifndef HEADER_SDLMS_INPUT
#define HEADER_SDLMS_INPUT

#include <SDL2/SDL.h>
#include <map>
#include <functional>

#include "util/currenton.hpp"

class Input : public Currenton<Input>
{
public:
	Input();

	void beginNewFrame();
	void keyUpEvent(const SDL_Event &event);
	void keyDownEvent(const SDL_Event &event);

	bool wasKeyPressed(SDL_Scancode key);
	bool wasKeyReleased(SDL_Scancode key);
	bool isKeyHeld(SDL_Scancode key);

	void mouseUpEvent(int key);
	void mouseDownEvent(int key);
	bool isMouseHeld(int key);
	bool wasMouseReleased(int key);

	int loop();
	void event(std::function<void(SDL_Event &event)> func);

private:
	std::map<SDL_Scancode, bool> _heldKeys;
	std::map<SDL_Scancode, bool> _pressedKeys;
	std::map<SDL_Scancode, bool> _releasedKeys;

	std::map<int, bool> _heldMouse;
	std::map<int, bool> _pressedMouse;
	std::map<int, bool> _releasedMouse;

private:
	std::vector<std::function<void(SDL_Event &event)>> _listeners;
};

#endif
