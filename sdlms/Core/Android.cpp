#ifdef __ANDROID__

#include <android/asset_manager.h>
#include "Android.h"

unsigned char * Android::get_file_buffer(const char *file){
    SDL_RWops* f = SDL_RWFromFile(file, "rb");
    auto asset=(AAsset*)f->hidden.androidio.asset;
    unsigned char* buff = (unsigned char *) AAsset_getBuffer(asset);
    return buff;
}

unsigned int Android::get_file_size(const char *file) {
    SDL_RWops* f = SDL_RWFromFile(file, "rb");
    auto asset=(AAsset*)f->hidden.androidio.asset;
    int length = AAsset_getLength(asset);
    return length;
}

#endif