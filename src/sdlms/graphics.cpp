
#include <SDL2/SDL.h>

#include "sdlms/graphics.hpp"
#include "graphics.hpp"

/* Graphics class
 * Holds all information dealing with graphics for the game
 */

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

Graphics::Graphics()
{
	if (SDL_Init(SDL_INIT_VIDEO) >= 0)
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
	// SDL_RenderCopy(this->_renderer, texture, sourceRectangle, destinationRectangle);
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
