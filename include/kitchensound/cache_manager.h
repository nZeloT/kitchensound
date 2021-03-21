#ifndef KITCHENSOUND_CACHE_MANAGER_H
#define KITCHENSOUND_CACHE_MANAGER_H

#include <string>
#include <memory>
#include <filesystem>

class FdRegistry;

class ResourceManager;

typedef void CURLM;
typedef void CURL;
typedef int curl_socket_t;

//Backbone of ResourceManager
//Goal: Load missing resources from their given destination to the filesystem and notify the resource manager
//      about the newly available resource
class CacheManager {
public:
    CacheManager(std::unique_ptr<FdRegistry>&, ResourceManager &, std::filesystem::path &);
    ~CacheManager();

    void load_from_cache(std::string const &url);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
    friend int curl_multi_timer_cb(CURLM*, long, Impl*);
    friend int curl_socket_cb(CURL*, curl_socket_t, int, void*, void*);
    friend struct _Fetch;
    friend struct _SocketInfo;
};

#endif //KITCHENSOUND_CACHE_MANAGER_H
