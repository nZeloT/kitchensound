#ifndef KITCHENSOUND_SNAPCAST_CONTROLLER_H
#define KITCHENSOUND_SNAPCAST_CONTROLLER_H

#include <memory>

#include "kitchensound/config.h"

class FdRegistry;

class SnapcastController {
public:
    SnapcastController(std::unique_ptr<FdRegistry>&, Configuration::SnapcastConfig);
    ~SnapcastController();

    void start_snapclient_service();
    void stop_snapclient_service();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif //KITCHENSOUND_SNAPCAST_CONTROLLER_H
