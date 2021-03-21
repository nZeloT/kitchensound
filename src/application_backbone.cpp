#include "kitchensound/application_backbone.h"

#include "kitchensound/config.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/fd_registry.h"
#include "kitchensound/renderer.h"


ApplicationBackbone::ApplicationBackbone(const std::filesystem::path& config_file)
    : conf{std::make_unique<Configuration>(config_file)}, fdreg{std::make_unique<FdRegistry>()},
      res{std::make_unique<ResourceManager>(fdreg, conf->get_res_folder(), conf->get_cache_folder())},
      rend{std::make_unique<Renderer>(res)},
      ctrl{std::make_unique<StateController>()} {};

ApplicationBackbone::~ApplicationBackbone() = default;