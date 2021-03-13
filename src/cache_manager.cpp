#include "kitchensound/cache_manager.h"

#include <fstream>

#include <spdlog/spdlog.h>
#include <SDL.h>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include "kitchensound/resource_manager.h"

static bool fetcher_active = true;

static std::string fetch_file(std::string const &url, std::filesystem::path& cache_root) {
    auto url_hash = std::hash<std::string>{}(url);
    auto cache_id = std::string{cache_root.string() + std::to_string(url_hash)};

    SPDLOG_INFO("Try'in to fetch from `{0}` with hash `{1}` into cache id `{2}`", url, url_hash, cache_id);

    bool failed = true;
    try {
        std::ofstream cache_file;
        cache_file.open(cache_id);

        curlpp::Easy request;
        request.setOpt(new curlpp::options::WriteStream(&cache_file));
        request.setOpt(new curlpp::options::Url(url));
        request.perform();

        cache_file.close();

        failed = false;
    } catch (curlpp::RuntimeError& error) {
        SPDLOG_ERROR("Received runtime error -> {0}", error.what());
    } catch (curlpp::LogicError& error) {
        SPDLOG_ERROR("Received logic error -> {0}", error.what());
    }

    if (failed)
        return std::string{};
    else
        return cache_id;
}

static int image_fetcher(void *cache_mgr) {
    auto cache = reinterpret_cast<CacheManager *>(cache_mgr);
    SPDLOG_INFO("Launched and active.");
    while (fetcher_active) {
        if (cache->_to_load.empty()) {
            SDL_Delay(2048);
            continue;
        }

        auto next_url = cache->_to_load.front();
        auto cache_id = std::move(fetch_file(next_url, cache->_cache_root));
        if (!cache_id.empty())
            cache->load_success(next_url, cache_id); //load the newly available resource through the resource manager
        else
            cache->load_failed(next_url);
        cache->_to_load.pop();
    }
    SPDLOG_INFO("Terminated.");
    return 0;
}

CacheManager::CacheManager(ResourceManager &res, std::filesystem::path& cache_root)
        : _res{res}, _cache_root{cache_root} {
    std::filesystem::create_directory(cache_root);
    SPDLOG_INFO("Launching fetcher thread.");
    _fetcher_thread = SDL_CreateThread(image_fetcher, "Image Fetcher", reinterpret_cast<void *>(this));
}

CacheManager::~CacheManager() {
    fetcher_active = false;
    int rval;
    SPDLOG_INFO("Waiting for the fetcher thread to terminate.");
    SDL_WaitThread(_fetcher_thread, &rval);
}

void CacheManager::schedule_for_fetch(const std::string &url) {
    auto _url = std::string {url};
    SPDLOG_INFO("Adding to fetch queue -> {}", _url);
    _to_load.push(_url);
}

void CacheManager::load_failed(const std::string &url) {
    _res.cache_load_failed(url);
}

void CacheManager::load_success(const std::string &url, std::string const &cache_id) {
    _res.cache_load_success(url, ResourceManager::load_image_raw(cache_id));
}