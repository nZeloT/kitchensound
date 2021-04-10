#include "kitchensound/resource_manager.h"

#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "kitchensound/cache_manager.h"

ResourceManager::ResourceManager(std::unique_ptr<NetworkController>& net, std::filesystem::path  res_root, std::filesystem::path  cache_root)
 : _cache(), _res_root{std::move(res_root)}, _cache_root{std::move(cache_root)}, _empty_cb{[](auto s, auto p){}},
   _net{net} {
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
    load_image("img/radio.png", _res_root.string() + "img/radio.png");
    load_image("img/arrow_left.png", _res_root.string() +"img/arrow_left.png");
    load_image("img/arrow_right.png", _res_root.string() + "img/arrow_right.png");
    load_image("img/bluetooth.png",  _res_root.string() +"img/bluetooth.png");
    load_image("img/gears.png", _res_root.string() + "img/gears.png");
    load_image("img/favorite.png", _res_root.string() + "img/favorite.png");
    load_image("img/speaker_group.png", _res_root.string() + "img/speaker_group.png");

    load_font("SMALL", _res_root.string() + "font/DroidSans.ttf", 18);
    load_font("LARGE", _res_root.string() + "font/DroidSans.ttf", 24);
    load_font("HUGHE", _res_root.string() + "font/DroidSans.ttf", 90);
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
        SPDLOG_WARN("Failed to load image -> {0}", SDL_GetError());
    return reinterpret_cast<void *>(image);
}

void *ResourceManager::load_font_raw(const std::string &path, int size) {
    if (size <= 0)
        throw std::runtime_error("Requested Font size smaller or equal to 0!");

    auto font = TTF_OpenFont(path.c_str(), size);
    if (font == nullptr) {
        SPDLOG_WARN("Font loading error -> {0}", SDL_GetError());
    } else {
        TTF_SetFontKerning(font, SDL_ENABLE);
    }

    return reinterpret_cast<void *>(font);
}

void ResourceManager::load_image(const std::string &identifier, const std::string &path, bool is_static) {
    auto image = load_image_raw(path);
    if (image == nullptr) {
        throw std::runtime_error("Failed to load static image! -> " + path);
    }

    if (is_static)
        _static.emplace(std::string{identifier},
                        Resource{IMAGE, LOADED, std::time(nullptr), reinterpret_cast<void *>(image), _empty_cb});
    else
        _cached.emplace(std::string{identifier},
                        Resource{IMAGE, LOADED, std::time(nullptr), reinterpret_cast<void *>(image), _empty_cb});
}

void ResourceManager::load_font(const std::string &identifier, const std::string &path, int size, bool is_static) {
    auto font = load_font_raw(path, size);
    if (font == nullptr) {
        throw std::runtime_error("Failed to load static font! -> " + path);
    }

    if (is_static)
        _static.emplace(std::string{identifier},
                        Resource{FONT, LOADED, std::time(nullptr), reinterpret_cast<void *>(font), _empty_cb});
    else
        _cached.emplace(std::string{identifier},
                        Resource{FONT, LOADED, std::time(nullptr), reinterpret_cast<void *>(font), _empty_cb});
}

void ResourceManager::get_cached(const std::string &identifier, std::function<void(std::string const&, void*)> image_cb) {
    auto _id = std::string{identifier};
    if(_id.empty()){
        image_cb(_id, nullptr);
        return;
    }

    auto loaded = get(_cached, _id);
    if (loaded == nullptr) {
        //not found means it isn't even scheduled for loading
        try_load_cached(_id, std::move(image_cb));
        return;
    }

    if(loaded->state == FAILED && std::time(nullptr) - loaded->last_state_upd > 80000) { // TODO use timer?
        //retry fetching the resource
        retry_load_cached(_id, std::move(image_cb));
        return;
    }

    image_cb(_id, loaded->data);
}

void ResourceManager::try_load_cached(const std::string &identifier, std::function<void(std::string const&, void*)> image_cb) {
    if(!_cache)
        _cache = std::make_unique<CacheManager>(_net, *this, _cache_root);
    SPDLOG_INFO("Try loading -> {0}", identifier);
    _cached.emplace(std::string{identifier}, Resource{.type = IMAGE, .state = SHEDULED, .data = nullptr, .cb = std::move(image_cb)});
    _cache->load_from_cache(std::string{identifier});
}

void ResourceManager::retry_load_cached(const std::string &identifier, std::function<void(std::string const&, void*)> image_cb) {
    auto r = _cached.find(identifier);
    if (r == std::end(_cached))
        return;
    auto id = std::string{identifier};
    SPDLOG_INFO("Retry loading -> {0}", id);
    r->second.state = SHEDULED;
    r->second.last_state_upd = std::time(nullptr);
    r->second.cb = std::move(image_cb);
    _cache->load_from_cache(id);
}

void ResourceManager::cache_load_failed(const std::string &identifier) {
    auto r = _cached.find(identifier);
    if (r == std::end(_cached))
        return;

    r->second.state = FAILED;
    r->second.last_state_upd = std::time(nullptr);
    r->second.cb(identifier, nullptr);
    r->second.cb = _empty_cb;
}

void ResourceManager::cache_load_success(const std::string &identifier, void *data) {
    auto r = _cached.find(identifier);
    if (r == std::end(_cached))
        return;

    r->second.state = LOADED;
    r->second.last_state_upd = std::time(nullptr);
    r->second.data = data;
    r->second.cb(identifier, data);
    r->second.cb = _empty_cb;
}