#include "kitchensound/resource_manager.h"

#include <memory>
#include <string>
#include <ctime>

#include <spdlog/spdlog.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "kitchensound/cache_manager.h"

ResourceManager::ResourceManager()
 : _cache() {
    load_all_static();
}

ResourceManager::~ResourceManager()  {
    unload_all();
}

ResourceManager::Resource const *ResourceManager::get(std::map<std::string, Resource> const &m, const std::string &s) {
    auto r = m.find(s);
    if (r == std::end(m))
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
    auto unload = [](std::map<std::string, Resource> &m) {
        for (auto &e : m) {
            auto &res = e.second;
            if (res.type == FONT)
                SDL_free(res.data);
            else
                SDL_FreeSurface(reinterpret_cast<SDL_Surface *>(res.data));
        }
        m.clear();
    };

    unload(_static);
    unload(_cached);
}

void *ResourceManager::load_image_raw(const std::string &path) {
    auto image = IMG_Load(path.c_str());
    if (image == nullptr)
        spdlog::warn("ResourceManager::load_image_raw(): IMG_Load(): {0}", SDL_GetError());
    return reinterpret_cast<void *>(image);
}

void *ResourceManager::load_font_raw(const std::string &path, int size) {
    if (size <= 0)
        throw std::runtime_error("Requested Font size smaller or equal to 0!");

    auto font = TTF_OpenFont(path.c_str(), size);
    if (font == nullptr) {
        spdlog::warn("ResourceManager::load_font_raw(): TTF_OpenFont(): {0}", SDL_GetError());
    } else {
        TTF_SetFontKerning(font, SDL_ENABLE);
    }

    return reinterpret_cast<void *>(font);
}

void ResourceManager::load_image(const std::string &identifier, const std::string &path, bool is_static) {
    auto image = load_image_raw(path);
    if (image == nullptr) {
        spdlog::error("ResourceManager::load_image(): Failed to load static image; Quitting!", SDL_GetError());
        throw std::runtime_error("Error loading radio image!");
    }

    if (is_static)
        _static.emplace(std::string{identifier},
                        Resource{IMAGE, LOADED, std::time(nullptr), reinterpret_cast<void *>(image)});
    else
        _cached.emplace(std::string{identifier},
                        Resource{IMAGE, LOADED, std::time(nullptr), reinterpret_cast<void *>(image)});
}

void ResourceManager::load_font(const std::string &identifier, const std::string &path, int size, bool is_static) {
    auto font = load_font_raw(path, size);
    if (font == nullptr) {
        spdlog::error("ResourceManager::load_font(): Failed to load static font!", SDL_GetError());
        throw std::runtime_error("Error loading font");
    }

    if (is_static)
        _static.emplace(std::string{identifier},
                        Resource{FONT, LOADED, std::time(nullptr), reinterpret_cast<void *>(font)});
    else
        _cached.emplace(std::string{identifier},
                        Resource{FONT, LOADED, std::time(nullptr), reinterpret_cast<void *>(font)});
}

void *ResourceManager::get_cached(const std::string &identifier) {
    auto _id = std::string{identifier};
    if(_id.empty())
        return nullptr;

    auto loaded = get(_cached, _id);
    if (loaded == nullptr) {
        //not found means it isn't even scheduled for loading
        try_load_cached(_id);
        return nullptr;
    }

    if(loaded->state == FAILED && std::time(nullptr) - loaded->last_state_upd > 80000) {
        //retry fetching the resource
        retry_load_cached(_id);
    }

    return loaded->data;
}

void ResourceManager::try_load_cached(const std::string &identifier) {
    if(!_cache)
        _cache = std::make_unique<CacheManager>(*this);
    spdlog::info("ResourceManager::try_load_cached(): Try loading `{}`", identifier);
    _cached.emplace(std::string{identifier}, Resource{.type = IMAGE, .state = SHEDULED, .data = nullptr});
    _cache->schedule_for_fetch(std::string{identifier});
}

void ResourceManager::retry_load_cached(const std::string &identifier) {
    auto r = _cached.find(identifier);
    if (r == std::end(_cached))
        return;
    auto id = std::string{identifier};
    spdlog::info("ResourceManager::retry_load_cached(): retry loading `{}`", id);
    r->second.state = SHEDULED;
    r->second.last_state_upd = std::time(nullptr);
    _cache->schedule_for_fetch(id);
}

void ResourceManager::cache_load_failed(const std::string &identifier) {
    auto r = _cached.find(identifier);
    if (r == std::end(_cached))
        return;

    r->second.state = FAILED;
    r->second.last_state_upd = std::time(nullptr);
}

void ResourceManager::cache_load_success(const std::string &identifier, void *data) {
    auto r = _cached.find(identifier);
    if (r == std::end(_cached))
        return;

    r->second.state = LOADED;
    r->second.last_state_upd = std::time(nullptr);
    r->second.data = data;
}