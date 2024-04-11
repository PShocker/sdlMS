
#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"

/* Graphics class
 * Holds all information dealing with graphics for the game
 */

Graphics::Graphics()
{
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) >= 0)
	{
		// 创建窗口
		_window = SDL_CreateWindow("sdlMS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		// 创建渲染器
		_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
		//禁用垂直同步
		SDL_GL_SetSwapInterval(0);
	}
}

Graphics::~Graphics()
{
	SDL_DestroyWindow(this->_window);
	SDL_DestroyRenderer(this->_renderer);
}

void Graphics::blitSurface(SDL_Texture *texture, SDL_Rect *sourceRectangle, SDL_FRect *destinationRectangle)
{
	SDL_RenderCopyF(this->_renderer, texture, sourceRectangle, destinationRectangle);
}

void Graphics::blitSurfaceEx(SDL_Texture *texture, SDL_Rect *sourceRectangle, SDL_FRect *destinationRectangle, const double angle, const SDL_FPoint *center, SDL_RendererFlip flip = SDL_FLIP_NONE)
{
	SDL_RenderCopyExF(this->_renderer, texture, sourceRectangle, destinationRectangle, angle, center, flip);
}

void Graphics::flip()
{
	SDL_RenderPresent(this->_renderer);
}

void Graphics::clear()
{
	SDL_RenderClear(this->_renderer);
}

SDL_Renderer *Graphics::getRenderer() const
{
	return this->_renderer;
}
