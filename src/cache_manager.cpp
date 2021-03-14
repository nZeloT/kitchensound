#include "kitchensound/cache_manager.h"

#include <fstream>
#include <queue>

#include <spdlog/spdlog.h>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Multi.hpp>
#include <curlpp/Exception.hpp>

#include "kitchensound/timeouts.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/timer_manager.h"
#include "kitchensound/timer.h"

static std::string fetch_file(std::string const &url, std::filesystem::path &cache_root) {
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
    } catch (curlpp::RuntimeError &error) {
        SPDLOG_ERROR("Received runtime error -> {0}", error.what());
    } catch (curlpp::LogicError &error) {
        SPDLOG_ERROR("Received logic error -> {0}", error.what());
    }

    if (failed)
        return std::string{};
    else
        return cache_id;
}

struct CacheRes {
    std::ofstream cache_file;
    std::string url;
    curlpp::Easy request;
};

struct CacheManager::Impl {
    Impl(TimerManager &tm, ResourceManager &res, std::filesystem::path &cache_root)
            : _res{res}, _cache_root{cache_root}, _curl_timer{tm.request_timer(CACHE_CURL_DELAY, true, [this]() {
        this->update();
    })} {
        std::filesystem::create_directory(cache_root);
    };

    ~Impl() = default;

    void update() {
        int leftRequests;
        if(!_curl_multi.perform(&leftRequests))
            _curl_timer.stop();
    }

    void image_fetcher() {
        auto next_url = this->_to_load.front();
        auto cache_id = std::move(fetch_file(next_url, this->_cache_root));
        if (!cache_id.empty())
            this->load_success(next_url, cache_id); //load the newly available resource through the resource manager
        else
            this->load_failed(next_url);
        this->_to_load.pop();
    }

    void shedule_for_fetch(const std::string &url) {
        SPDLOG_INFO("Adding to fetch queue -> {}", url);

        auto url_hash = std::hash<std::string>{}(url);
        auto cache_id = std::string{_cache_root.string() + std::to_string(url_hash)};

        SPDLOG_INFO("Fetching into -> {0}", cache_id);

        CacheRes r;
        r.url = std::string{url};
        r.cache_file.open(cache_id);
        r.request.setOpt(new curlpp::options::Url(r.url));
        r.request.setOpt(new curlpp::options::WriteStream(&r.cache_file));

        _curl_multi.add(&r.request);
        _requests.push(std::move(r));
    }

    void load_failed(const std::string &url) {
        _res.cache_load_failed(url);
    }

    void load_success(const std::string &url, std::string const &cache_id) {
        _res.cache_load_success(url, ResourceManager::load_image_raw(cache_id));
    }

    curlpp::Cleanup _curl_cleanup;
    curlpp::Multi _curl_multi;
    Timer &_curl_timer;

    std::queue<CacheRes> _requests;
    ResourceManager &_res;
    std::filesystem::path _cache_root;
};

CacheManager::CacheManager(TimerManager &tm, ResourceManager &res, std::filesystem::path &cache_root)
        : _impl{std::make_unique<Impl>(tm, res, cache_root)} {}

CacheManager::~CacheManager() = default;

void CacheManager::schedule_for_fetch(const std::string &url) {
    _impl->shedule_for_fetch(url);
}