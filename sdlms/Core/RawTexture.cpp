#include "RawTexture.h"
#include "Window.h"

std::vector<u8> *RawTexture::load(wz::Property<wz::WzCanvas> *canvas)
{
    static std::unordered_map<wz::Property<wz::WzCanvas> *, std::vector<u8>> cache;
    if (cache.contains(canvas))
    {
        return &cache[canvas];
    }
    else
    {
        cache[canvas] = canvas->get_raw_data();
        return &cache[canvas];
    }
}