#ifndef KITCHENSOUND_STATION_IMAGE_CACHE_H
#define KITCHENSOUND_STATION_IMAGE_CACHE_H

#include <vector>
#include <map>

#include <SDL.h>

#include "kitchensound/model.h"

//class ImageCache {
//public:
//    explicit ImageCache(std::vector<RadioStationStream> &stations);
//    ~ImageCache();
//
//    SDL_Surface* get_or_fetch_image(std::string const& image_url);
//
//private:
//    enum IMAGE_STATE {
//        UNKNOWN,
//        LOADED,
//        FETCHING
//    };
//
//    struct Image {
//        std::string file;
//        IMAGE_STATE state;
//        SDL_Surface* texture;
//        std::string image_url;
//    };
//
//    void init_image(std::string const& image_url);
//
//    std::map<std::string, Image> _cache;
//
//};

#endif //KITCHENSOUND_STATION_IMAGE_CACHE_H
