#ifndef HEADER_SDLMS_GRAPHICS
#define HEADER_SDLMS_GRAPHICS

#include <SDL2/SDL.h>

#include <map>
#include <string>
#include <vector>
#include "template/currenton.hpp"


class Graphics final : public Currenton<Graphics>
{
public:
	Graphics();
	~Graphics();

	/* void blitSurface
	 * Draws a texture to a certain part of the screen
	 */
	void blitSurface(SDL_Texture *source, SDL_Rect *sourceRectangle, SDL_FRect *destinationRectangle);

	/* void blitSurfaceEx
	 * Draws a texture to a certain part of the screen with flip
	 */
	void blitSurfaceEx(SDL_Texture *source, SDL_Rect *sourceRectangle, SDL_FRect *destinationRectangle, double angle, const SDL_FPoint *center, SDL_RendererFlip flip);
	/* void flip
	 * Renders everything to the screen
	 */
	void flip();

	/* void clear
	 * Clears the screen
	 */
	void clear();

	/* SDL_Renderer* getRenderer
	 * Returns the renderer
	 */
	SDL_Renderer *getRenderer() const;

public:
	static const int SCREEN_WIDTH = 1366;
	static const int SCREEN_HEIGHT = 768;

	// static const int SCREEN_WIDTH = 1920;
	// static const int SCREEN_HEIGHT = 1080;

	// static const int SCREEN_WIDTH = 800;
	// static const int SCREEN_HEIGHT = 600;

private:
	SDL_Window *_window;
	SDL_Renderer *_renderer;
};

#endif
