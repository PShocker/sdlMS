#ifndef HEADER_SDLMS_GRAPHICS
#define HEADER_SDLMS_GRAPHICS

#include <SDL2/SDL.h>

#include <map>
#include <string>
#include <vector>
#include "util/currenton.hpp"
#include "sdlms/tile.hpp"

class Graphics final : public Currenton<Graphics>
{
public:
	Graphics();
	~Graphics();

	/* void blitSurface
	 * Draws a texture to a certain part of the screen
	 */
	void blitSurface(SDL_Texture *source, SDL_Rect *sourceRectangle, SDL_Rect *destinationRectangle);

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

	void set_tile(std::vector<Tile> tile);

	std::vector<Tile> get_tile();

private:
	SDL_Window *_window;
	SDL_Renderer *_renderer;

	std::map<std::string, SDL_Surface *> _spriteSheets;
	std::vector<Tile> _tile;
};

#endif
