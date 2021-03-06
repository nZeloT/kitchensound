#ifndef KITCHENSOUND_CACHE_MANAGER_H
#define KITCHENSOUND_CACHE_MANAGER_H

#include <string>
#include <queue>
#include <filesystem>

struct SDL_Thread;
class ResourceManager;

static int image_fetcher(void* cache_mgr);

//Backbone of ResourceManager
//Goal: Load missing resources from their given destination to the filesystem and notify the resource manager
//      about the newly available resource
class CacheManager {
public:
    CacheManager(ResourceManager &res, std::filesystem::path &cache_root);
    ~CacheManager();

    void schedule_for_fetch(std::string const &url);

private:
    friend int image_fetcher(void* cache_mgr);

    void load_success(std::string const& url, std::string const& cache_id);
    void load_failed(std::string const& url);

    SDL_Thread* _fetcher_thread;
    std::queue<std::string> _to_load;
    ResourceManager &_res;
    std::filesystem::path _cache_root;
};

#endif //KITCHENSOUND_CACHE_MANAGER_H
