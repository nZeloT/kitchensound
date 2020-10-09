#include "kitchensound/resource_manager.h"

#include <string>

#include <spdlog/spdlog.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

static bool cached_loader_enabled = true;
static int cached_loader(void* dataptr) {
    auto res_mgr = reinterpret_cast<ResourceManager*>(dataptr);

    while(cached_loader_enabled) {
        if(!res_mgr->_to_load.empty()) {
            auto next = res_mgr->_to_load.front();

            auto resource_pair = res_mgr->_cached.find(next);
            if(resource_pair == std::end(res_mgr->_cached)) {
                spdlog::warn("Couldn't find a cached resource for entry: '{1}'", next);
                continue;
            }
            auto resource = resource_pair->second;

            if(resource.state == ResourceManager::SHEDULED) {
                void* data = nullptr;
                if(resource.type == ResourceManager::IMAGE)
                    data = ResourceManager::load_image_raw(next);
                else
                    spdlog::warn("Tried to dynamically load a font; This is not supported!");

                if(data == nullptr) {
                    res_mgr->_cached.erase(resource_pair);
                }else {
                    //place the newly loaded image into the dynamic cache
                    resource_pair->second.state = ResourceManager::LOADED;
                    resource_pair->second.data  = data;
                }
            }

            res_mgr->_to_load.pop();
        }
        SDL_Delay(512);
    }

    return 0;
}

ResourceManager::Resource const* ResourceManager::get(std::map<std::string, Resource> const& m, const std::string &s) {
    auto r = m.find(s);
    if(r == std::end(m))
        return nullptr;

    return &(r->second);
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

void* ResourceManager::load_image_raw(const std::string &path) {
    auto image = IMG_Load(path.c_str());
    if(image == nullptr)
        spdlog::warn("IMG_Load(): {1}", SDL_GetError());
    return reinterpret_cast<void*>(image);
}

void* ResourceManager::load_font_raw(const std::string &path, int size) {
    if(size <= 0)
        throw std::runtime_error("Requested Font size smaller or equal to 0!");

    auto font = TTF_OpenFont(path.c_str(), size);
    if (font == nullptr) {
        spdlog::warn("TTF_OpenFont(): {1}", SDL_GetError());
    }else {
        TTF_SetFontKerning(font, SDL_ENABLE);
    }

    return reinterpret_cast<void*>(font);
}

void ResourceManager::load_image(const std::string &identifier, const std::string &path, bool is_static) {
    auto image = load_image_raw(path);
    if (image == nullptr) {
        spdlog::error("Failed to load static image; Quitting!", SDL_GetError());
        throw std::runtime_error("Error loading radio image!");
    }

    if(is_static)
        _static.emplace(std::string{identifier}, Resource{IMAGE, LOADED, reinterpret_cast<void*>(image)});
    else
        _cached.emplace(std::string{identifier}, Resource{IMAGE, LOADED, reinterpret_cast<void*>(image)});
}

void ResourceManager::load_font(const std::string &identifier, const std::string &path, int size, bool is_static) {
    auto font = load_font_raw(path, size);
    if (font == nullptr) {
        spdlog::error("Failed to load static font!", SDL_GetError());
        throw std::runtime_error("Error loading font");
    }

    if(is_static)
        _static.emplace(std::string{identifier}, Resource{FONT, LOADED, reinterpret_cast<void*>(font)});
    else
        _cached.emplace(std::string{identifier}, Resource{FONT, LOADED, reinterpret_cast<void*>(font)});
}

void * ResourceManager::get_cached(const std::string &identifier) {
    auto loaded = get(_cached, identifier);
    if(loaded == nullptr) {
        //not found means it isn't even scheduled for loading
        try_load_cached(identifier);
    }

    return loaded->data;
}

void ResourceManager::try_load_cached(const std::string &identifier) {
    _cached.emplace(std::string{identifier}, Resource{ .type = IMAGE, .state = SHEDULED, .data = nullptr });
    _to_load.push(std::string{identifier});
    // TODO: also somehow make sure that the memory doesn't fill its only 512 mb
    //       therefore use an access counter + last access time to identify resources which can safely be unloaded
}