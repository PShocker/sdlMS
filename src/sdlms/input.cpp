#include "sdlms/input.hpp"
#include "input.hpp"

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

	this->_pressedMouse.clear();
	this->_releasedMouse.clear();
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

void Input::mouseDownEvent(int key)
{
	this->_pressedMouse[key] = true;
	this->_heldMouse[key] = true;
}

void Input::mouseUpEvent(int key)
{
	this->_releasedMouse[key] = true;
	this->_heldMouse[key] = false;
}

bool Input::isMouseHeld(int key)
{
	return this->_heldMouse[key];
}
// Check if a certain key was released during the current frame
bool Input::wasMouseReleased(int key)
{
	return this->_releasedMouse[key];
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
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			mouseDownEvent(event.button.button);
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			mouseUpEvent(event.button.button);
		}
		else if (event.type == SDL_QUIT)
		{
			return -1;
		}
	}

	for (auto &[_, listener] : _listeners)
	{
		listener(event);
	}

	return 0;
}

void Input::event(std::u16string key, std::function<void(SDL_Event &event)> func)
{
	_listeners[key] = func;
}

void Input::remove_event(std::u16string key)
{
	if (_listeners.contains(key))
	{
		_listeners.erase(key);
	}
}

void Input::check()
{
	// 解决按键冲突
	if (isKeyHeld(SDL_SCANCODE_RIGHT))
	{
		// 按住右键,则左键无效
		this->_heldKeys[SDL_SCANCODE_LEFT] = false;
	}
	if (isKeyHeld(SDL_SCANCODE_UP))
	{
		// 按住上键,则下键无效
		this->_heldKeys[SDL_SCANCODE_DOWN] = false;
	}
}
