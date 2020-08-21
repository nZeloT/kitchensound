#ifndef KITCHENSOUND_RESOURCE_MANAGER_H
#define KITCHENSOUND_RESOURCE_MANAGER_H

#include <string>
#include <map>

class ResourceManager {
public:
    ResourceManager() {
        load_all_static();
    };
    ~ResourceManager() {
        unload_all();
    }
    void* get_static(std::string const& name) { return get(_static, name); };
    void* get_cached(std::string const& identifier);

private:
    enum ResourceType {
        FONT,
        IMAGE
    };

    struct Resource {
        ResourceType type;
        void* data;
    };

    static void* get(std::map<std::string, Resource> const& m, std::string const& s);
    void load_all_static();
    void load_image(std::string const& identifier, std::string const& path, bool is_static = true);
    void load_font(std::string const& identifier, std::string const& path, int size, bool is_static = true);
    void try_load_cached(std::string const& identifier);
    void unload_all();

    std::map<std::string, Resource> _static;
    std::map<std::string, Resource> _cached;
};

#endif //KITCHENSOUND_RESOURCE_MANAGER_H
