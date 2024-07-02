#include "Window.h"

#include <SDL2/SDL.h>

#include "Core/ECSSystem.h"

#include <cstdlib>

void Window::create_window(const char *title, unsigned int width, unsigned int height, unsigned int pixel_scale_h, unsigned int pixel_scale_v)
{
	int result = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
	[[unlikely]]
	if (result < 0)
	{
		ECS_PRINT_ERROR("Failed to initialize SDL (%s)", SDL_GetError());
		std::abort();
	}
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	[[unlikely]]
	if (!window)
	{
		ECS_PRINT_ERROR("Failed to create SDL window (%s)", SDL_GetError());
		std::abort();
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_RenderSetLogicalSize(renderer, width, height);

	window_title = title;
}

void Window::destroy_window()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

SDL_Window *Window::get_main_window()
{
	return window;
}

SDL_Renderer *Window::get_renderer()
{
	return renderer;
}

const char *Window::get_title()
{
	return window_title;
}

SDL_Point Window::get_mouse_position()
{
	return {mouse_x, mouse_y};
}

void Window::clear()
{
	SDL_RenderClear(renderer);
}

void Window::update()
{
	SDL_GetMouseState(&mouse_x, &mouse_y);
	SDL_RenderPresent(renderer);
}
