#include "sdlms/input.hpp"

/* Input class
 * Keeps track of keyboard state
 */

Input::Input()
{
}

// This function gets called at the beginning of each new frame
// to reset the keys that are no longer relevant
void Input::beginNewFrame()
{
	this->_pressedKeys.clear();
	this->_releasedKeys.clear();
}

// This gets called when a key has been pressed
void Input::keyDownEvent(const SDL_Event &event)
{
	this->_pressedKeys[event.key.keysym.scancode] = true;
	this->_heldKeys[event.key.keysym.scancode] = true;
}

// This gets called when a key is released
void Input::keyUpEvent(const SDL_Event &event)
{
	this->_releasedKeys[event.key.keysym.scancode] = true;
	this->_heldKeys[event.key.keysym.scancode] = false;
}

// Check if a certain key was pressed during the current frame
bool Input::wasKeyPressed(SDL_Scancode key)
{
	return this->_pressedKeys[key];
}

// Check if a certain key was released during the current frame
bool Input::wasKeyReleased(SDL_Scancode key)
{
	return this->_releasedKeys[key];
}

// Check if a certain key is currently being held
bool Input::isKeyHeld(SDL_Scancode key)
{
	return this->_heldKeys[key];
}

int Input::loop()
{
	beginNewFrame();
	SDL_Event event;
	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_KEYDOWN)
		{
			if (event.key.repeat == 0)
			{
				keyDownEvent(event);
			}
		}
		else if (event.type == SDL_KEYUP)
		{
			keyUpEvent(event);
		}
		else if (event.type == SDL_QUIT)
		{
			return -1;
		}
	}

	for (auto &listener : _listeners)
	{
		listener(event);
	}

	return 0;
}

void Input::event(std::function<void(SDL_Event &event)> func)
{
	_listeners.push_back(func);
}
