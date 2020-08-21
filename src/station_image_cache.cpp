#include "kitchensound/station_image_cache.h"

#include <iostream>

//ImageCache::ImageCache(std::vector<RadioStationStream> &stations) {
//    std::cout << "ImageCache::ImageCache(): init textures" << std::endl;
//    for(auto& e : stations) {
//        if(!e.image_url.empty()){
//            _cache.emplace(std::string{e.image_url});
//        }
//    }
//}
//
//ImageCache::~ImageCache() {
//    std::cout << "ImageCache::~ImageCache(): freeing loaded textures" << std::endl;
//    for(auto& e : _cache) {
//        if(e.second.texture != nullptr)
//            SDL_FreeSurface(e.second.texture);
//    }
//    std::cout << "ImageCache::~ImageCache():: freed all textures" << std::endl;
//}
//
//void ImageCache::init_image(std::string const& image_url) {
//
//}
//
//SDL_Surface *ImageCache::get_or_fetch_image(std::string const& image_url) {
//    return nullptr;
//}
