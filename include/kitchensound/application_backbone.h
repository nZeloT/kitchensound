#ifndef KITCHENSOUND_APPLICATION_BACKBONE_H
#define KITCHENSOUND_APPLICATION_BACKBONE_H

#include <filesystem>
#include <memory>

class Configuration;
class StateController;
class ResourceManager;
class FdRegistry;
class Renderer;
class NetworkController;
class AnalyticsLogger;

struct ApplicationBackbone {
    explicit ApplicationBackbone(const std::filesystem::path& config_file);
    ~ApplicationBackbone();

    std::unique_ptr<Configuration> conf;
    std::unique_ptr<StateController> ctrl;
    std::unique_ptr<ResourceManager> res;
    std::unique_ptr<FdRegistry> fdreg;
    std::unique_ptr<Renderer> rend;
    std::unique_ptr<NetworkController> net;
    std::unique_ptr<AnalyticsLogger> analytics;
};


#endif //KITCHENSOUND_APPLICATION_BACKBONE_H
