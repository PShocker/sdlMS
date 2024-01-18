
#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"

/* Graphics class
 * Holds all information dealing with graphics for the game
 */

Graphics::Graphics()
{
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) >= 0)
	{
		SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &this->_window, &this->_renderer);
		SDL_SetWindowTitle(this->_window, "sdlMS");
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
