#include "kitchensound/cache_manager.h"

#include <fstream>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/resource_manager.h"
#include "kitchensound/network_controller.h"

struct CacheManager::Impl {

    Impl(std::unique_ptr<NetworkController>& net, ResourceManager &res, std::filesystem::path &cache_root)
            : _res{res}, _cache_root{cache_root}, _net{net} {
        std::filesystem::create_directory(cache_root);
    };

    ~Impl() = default;

    void load_from_cache(const std::string& url) {
        //check if already fetched, if not fetch it
        auto url_hash = std::hash<std::string>{}(url);
        auto cache_id = std::string{_cache_root.string() + std::to_string(url_hash)};

        if(std::filesystem::exists(std::filesystem::path{cache_id})){
            SPDLOG_DEBUG("Requested ressource found in cache -> {}; {}", cache_id, url);
            load_success(url, cache_id);
        }else{
            schedule_for_fetch(cache_id, url);
        }

    }

    void schedule_for_fetch(std::string const& cache_id, const std::string &url) {
        _net->add_request(url, HTTP_METHOD::GET, [this, cache_id](auto url, auto success, auto& response) {
            if(success) {
                std::ofstream out;
                out.open(cache_id);
                out.write(response.response, response.size);
                out.close();
                this->load_success(url, cache_id);
            }else{
                this->load_failed(url);
            }
        });
    }

    void load_failed(const std::string &url) {
        SPDLOG_DEBUG("Notifying Resourcemanager about failed load -> {}", url);
        _res.cache_load_failed(url);
    }

    void load_success(const std::string &url, const std::string &cache_id) {
        SPDLOG_DEBUG("Notifying Resourcemanager about successful load -> {}", url);
        _res.cache_load_success(url, ResourceManager::load_image_raw(cache_id));
    }

    ResourceManager &_res;
    std::unique_ptr<NetworkController>& _net;
    std::filesystem::path _cache_root;
};


CacheManager::CacheManager(std::unique_ptr<NetworkController>& net, ResourceManager &res, std::filesystem::path &cache_root)
        : _impl{std::make_unique<Impl>(net, res, cache_root)} {}

CacheManager::~CacheManager() = default;

void CacheManager::load_from_cache(const std::string &url) {
    _impl->load_from_cache(url);
}