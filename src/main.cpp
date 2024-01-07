#include <SDL2/SDL.h>
#include <stdio.h>

#include <wz/Node.hpp>
#include <wz/File.hpp>
#include <wz/Directory.hpp>
#include <wz/Property.hpp>

#include "util/string_util.hpp"
#include "util/wz_util.hpp"

// Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 800;

SDL_Rect camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

int main(int argc, char *args[])
{
    WzUtil::init();
    int mapId = 100000000;
    wz::Node *root = WzUtil::Map->get_root();
    auto node = root->find_from_path(WzUtil::get_map_node_path(mapId));

    // The window we'll be rendering to
    SDL_Window *window = NULL;
    // The surface contained by the window
    SDL_Surface *screenSurface = NULL;
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        // Create window
        window = SDL_CreateWindow("sdlMS", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else
        {
            SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
            for (size_t i = 0; i < 8; i++)
            {
                auto tile = node->get_child(StringUtil::to_ustring(std::to_string(i)));
                auto tsNode = tile->get_child(u"info")->get_child(u"tS");
                if (tsNode != nullptr)
                {
                    auto tileClass = dynamic_cast<wz::Property<wz::wzstring> *>(tsNode)->get();
                    auto tileNode = tile->get_child(u"tile");
                    for (auto it : tileNode->get_children())
                    {
                        auto u = dynamic_cast<wz::Property<wz::wzstring> *>(it.second[0]->get_child(u"u"))->get();
                        auto no = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"no"))->get();
                        auto url = u"Tile/" + tileClass + u".img/" + u + u"/" + StringUtil::to_ustring(std::to_string(no));

                        auto tile = root->find_from_path(url);
                        auto canvas = dynamic_cast<wz::Property<wz::WzCanvas> *>(tile);
                        auto raw_data = canvas->get_raw_data();
                        auto imageHeight = canvas->get().height;
                        auto imageWidth = canvas->get().width;

                        auto o = dynamic_cast<wz::Property<wz::WzVec2D> *>(tile->get_child(u"origin"));
                        auto ox = o->get().x;
                        auto oy = o->get().y;

                        auto z = dynamic_cast<wz::Property<int> *>(tile->get_child(u"z"))->get();
                        if (z == 0)
                        {
                            z = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"zM"))->get();
                        }

                        auto x = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"x"))->get();
                        auto y = dynamic_cast<wz::Property<int> *>(it.second[0]->get_child(u"y"))->get();

                        // SDL_Surface *imageSurface = SDL_CreateRGBSurfaceFrom(raw_data.data(), imageWidth, imageHeight, 16, imageWidth * 2,
                        //                                                      0x0F00, 0x00F0, 0x000F, 0xF000);
                        // auto fmt = imageSurface->format;
                        // SDL_Texture *texture2 = SDL_CreateTextureFromSurface(renderer, imageSurface);
                        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, imageWidth, imageHeight);
                        SDL_UpdateTexture(texture, NULL, raw_data.data(), imageWidth * sizeof(Uint16));
                        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
                        

                        SDL_Rect dstRect;
                        dstRect.x = x - ox;
                        dstRect.y = y - oy;
                        dstRect.w = imageWidth;
                        dstRect.h = imageHeight;

                        // SDL_RenderClear(renderer);
                        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
                        // break;
                    }
                }
            }
            SDL_RenderPresent(renderer);
            SDL_Event event;
            while (true)
            {
                if (SDL_PollEvent(&event))
                {
                    if (event.type == SDL_QUIT)
                    {
                        break;
                    }
                }
                // 处理输入
                const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
                if (keyboardState[SDL_SCANCODE_LEFT])
                {
                    camera.x -= 5;
                    // break;
                }
                if (keyboardState[SDL_SCANCODE_RIGHT])
                {
                    camera.x += 5;
                }
                if (keyboardState[SDL_SCANCODE_UP])
                {
                    camera.y -= 5;
                }
                if (keyboardState[SDL_SCANCODE_DOWN])
                {
                    camera.y += 5;
                }
                SDL_RenderSetViewport(renderer, &camera);

                // 更新屏幕
                SDL_RenderPresent(renderer);
                SDL_Delay(16);
                // SDL_RenderClear(renderer);
            }
        }
    }
    // Destroy window
    SDL_DestroyWindow(window);
    // Quit SDL subsystems
    SDL_Quit();
    return 0;
}