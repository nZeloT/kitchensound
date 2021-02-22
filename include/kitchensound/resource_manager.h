#ifndef KITCHENSOUND_RESOURCE_MANAGER_H
#define KITCHENSOUND_RESOURCE_MANAGER_H

#include <string>
#include <memory>
#include <map>
#include <queue>
#include <filesystem>
#include <functional>

class CacheManager;
class ResourceManager {
public:
    ResourceManager(std::filesystem::path  res_root, std::filesystem::path  cache_root);
    ~ResourceManager();
    void* get_static(std::string const& name) { return get(_static, name)->data; };
    void get_cached(std::string const& identifier, std::function<void(std::string const&, void*)>);

private:
    friend CacheManager;

    enum ResourceType {
        FONT,
        IMAGE
    };

    enum ResourceState {
        SHEDULED,
        LOADED,
        FAILED
    };

    struct Resource {
        ResourceType type;
        ResourceState state;
        long last_state_upd;
        void* data;
        std::function<void(std::string const&, void*)> cb;
    };

    static const Resource * get(std::map<std::string, Resource> const& m, std::string const& s);
    void load_all_static();
    void load_image(std::string const& identifier, std::string const& path, bool is_static = true);
    void load_font(std::string const& identifier, std::string const& path, int size, bool is_static = true);

    void try_load_cached(std::string const& identifier, std::function<void(std::string const&, void*)>);
    void retry_load_cached(std::string const& identifier, std::function<void(std::string const&, void*)>);
    void cache_load_failed(std::string const& identifier);
    void cache_load_success(std::string const& identifier, void* data);

    void unload_all();

    static void* load_image_raw(std::string const& path);
    static void* load_font_raw(std::string const& path, int size);

    std::map<std::string, Resource> _static;
    std::map<std::string, Resource> _cached;

    std::filesystem::path _res_root;
    std::filesystem::path _cache_root;

    std::function<void(std::string const&, void*)> _empty_cb;

    std::unique_ptr<CacheManager> _cache;
};

#endif //KITCHENSOUND_RESOURCE_MANAGER_H
