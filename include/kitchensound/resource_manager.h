#ifndef KITCHENSOUND_RESOURCE_MANAGER_H
#define KITCHENSOUND_RESOURCE_MANAGER_H

#include <string>
#include <map>
#include <queue>

static int cached_loader(void* dataptr);
class ResourceManager {
public:
    ResourceManager() {
        load_all_static();
    };
    ~ResourceManager() {
        unload_all();
    }
    void* get_static(std::string const& name) { return get(_static, name)->data; };
    void* get_cached(std::string const& identifier);

private:
    friend int cached_loader(void* dataptr);

    enum ResourceType {
        FONT,
        IMAGE
    };

    enum ResourceState {
        SHEDULED,
        LOADED
    };

    struct Resource {
        ResourceType type;
        ResourceState state;
        void* data;
    };

    static const Resource * get(std::map<std::string, Resource> const& m, std::string const& s);
    void load_all_static();
    void load_image(std::string const& identifier, std::string const& path, bool is_static = true);
    void load_font(std::string const& identifier, std::string const& path, int size, bool is_static = true);
    void try_load_cached(std::string const& identifier);
    void unload_all();

    static void* load_image_raw(std::string const& path);
    static void* load_font_raw(std::string const& path, int size);

    std::map<std::string, Resource> _static;
    std::map<std::string, Resource> _cached;
    std::queue<std::string> _to_load;
};

#endif //KITCHENSOUND_RESOURCE_MANAGER_H
