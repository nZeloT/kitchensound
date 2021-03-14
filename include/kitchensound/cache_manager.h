#ifndef KITCHENSOUND_CACHE_MANAGER_H
#define KITCHENSOUND_CACHE_MANAGER_H

#include <string>
#include <memory>
#include <filesystem>

class TimerManager;

class ResourceManager;

//Backbone of ResourceManager
//Goal: Load missing resources from their given destination to the filesystem and notify the resource manager
//      about the newly available resource
class CacheManager {
public:
    CacheManager(TimerManager&, ResourceManager &, std::filesystem::path &);
    ~CacheManager();

    void schedule_for_fetch(std::string const &url);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif //KITCHENSOUND_CACHE_MANAGER_H
