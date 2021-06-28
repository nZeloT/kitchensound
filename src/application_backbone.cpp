#include "kitchensound/application_backbone.h"

#include <spdlog/spdlog.h>

#include "kitchensound/config.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/fd_registry.h"
#include "kitchensound/renderer.h"
#include "kitchensound/network_controller.h"
#include "kitchensound/analytics_logger.h"


ApplicationBackbone::ApplicationBackbone(const std::filesystem::path &config_file)
        : conf{std::make_unique<Configuration>(config_file)},
          fdreg{std::make_unique<FdRegistry>()},
          net{std::make_unique<NetworkController>(fdreg)},
          res{std::make_unique<ResourceManager>(net, conf->get_res_folder(), conf->get_cache_folder())},
          rend{std::make_unique<Renderer>(res)},
          analytics{std::make_unique<AnalyticsLogger>(net, conf->get_analytics_config())},
          ctrl{std::make_unique<StateController>(analytics)} {};

ApplicationBackbone::~ApplicationBackbone() {
    SPDLOG_DEBUG("Begin dropping Application Backbone ...");
    ctrl.reset();
    analytics.reset();
    rend.reset();
    res.reset();
    net.reset();
    fdreg.reset();
    conf.reset();
    SPDLOG_DEBUG("Dropped ApplicationBackbone");
}