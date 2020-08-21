#include "kitchensound/resource_manager.h"

#include <string>

#include <spdlog/spdlog.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

void* ResourceManager::get(std::map<std::string, Resource> const& m, const std::string &s) {
    auto r = m.find(s);
    if(r == std::end(m))
        return nullptr;

    return r->second.data;
}

void ResourceManager::load_all_static() {
    load_image("img/radio.png", "../res/img/radio.png");
    load_image("img/arrow_left.png", "../res/img/arrow_left.png");
    load_image("img/arrow_right.png", "../res/img/arrow_right.png");
    load_image("img/bluetooth.png", "../res/img/bluetooth.png");

    load_font("SMALL", "../res/font/DroidSans.ttf", 18);
    load_font("LARGE", "../res/font/DroidSans.ttf", 24);
    load_font("HUGHE", "../res/font/DroidSans.ttf", 90);
}

void ResourceManager::unload_all() {
    auto unload = [](std::map<std::string, Resource>& m){
        for(auto& e : m) {
            auto& res = e.second;
            if(res.type == FONT)
                SDL_free(res.data);
            else
                SDL_FreeSurface(reinterpret_cast<SDL_Surface*>(res.data));
        }
        m.clear();
    };

    unload(_static);
    unload(_cached);
}

void ResourceManager::load_image(const std::string &identifier, const std::string &path, bool is_static) {
    auto image = IMG_Load("../res/img/radio.png");
    if (image == nullptr) {
        spdlog::error("IMG_Load(): {1}", SDL_GetError());
        throw std::runtime_error("Error loading radio image!");
    }

    if(is_static)
        _static.emplace(std::string{identifier}, Resource{IMAGE, reinterpret_cast<void*>(image)});
    else
        _cached.emplace(std::string{identifier}, Resource{IMAGE, reinterpret_cast<void*>(image)});
}

void ResourceManager::load_font(const std::string &identifier, const std::string &path, int size, bool is_static) {
    if(size <= 0)
        throw std::runtime_error("Requested Font size smaller or equal to 0!");

    auto font = TTF_OpenFont("../res/font/DroidSans.ttf", size);
    if (font == nullptr) {
        spdlog::error("TTF_OpenFont(): {1}", SDL_GetError());
        throw std::runtime_error("Error loading font 1");
    }
    TTF_SetFontKerning(font, SDL_ENABLE);

    if(is_static)
        _static.emplace(std::string{identifier}, Resource{FONT, reinterpret_cast<void*>(font)});
    else
        _cached.emplace(std::string{identifier}, Resource{FONT, reinterpret_cast<void*>(font)});
}

void * ResourceManager::get_cached(const std::string &identifier) {
    auto loaded = get(_cached, identifier);
    if(loaded == nullptr) {
        try_load_cached(identifier);
    }

    return loaded;
}

void ResourceManager::try_load_cached(const std::string &identifier) {

}