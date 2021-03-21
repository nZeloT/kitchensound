#ifndef KITCHENSOUND_APPLICATION_BACKBONE_H
#define KITCHENSOUND_APPLICATION_BACKBONE_H

#include <filesystem>
#include <memory>

class Configuration;
class StateController;
class ResourceManager;
class FdRegistry;
class Renderer;

struct ApplicationBackbone {
    explicit ApplicationBackbone(const std::filesystem::path& config_file);
    ~ApplicationBackbone();

    std::unique_ptr<Configuration> conf;
    std::unique_ptr<StateController> ctrl;
    std::unique_ptr<ResourceManager> res;
    std::unique_ptr<FdRegistry> fdreg;
    std::unique_ptr<Renderer> rend;
};


#endif //KITCHENSOUND_APPLICATION_BACKBONE_H
