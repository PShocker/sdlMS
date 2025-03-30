#pragma once

#include <string>
#include "Components/Components.h"

struct Button
{
    static void load(std::u16string path, std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> &pair);
    static void over(SDL_FRect &rect, std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> &pair);
    static void click(SDL_FRect &rect, std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> &pair, std::map<std::pair<std::u16string, std::unordered_map<std::u16string, AnimatedSprite>> *, std::function<void()>> click_map);
};