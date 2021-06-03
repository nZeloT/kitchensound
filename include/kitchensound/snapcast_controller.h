#ifndef KITCHENSOUND_SNAPCAST_CONTROLLER_H
#define KITCHENSOUND_SNAPCAST_CONTROLLER_H

#include <memory>
#include <functional>

#include "kitchensound/config.h"

struct Song;
class FdRegistry;
class AnalyticsLogger;

class SnapcastController {
public:
    SnapcastController(std::unique_ptr<FdRegistry>&, std::unique_ptr<AnalyticsLogger>&, Configuration::SnapcastConfig);
    ~SnapcastController();

    void start_snapclient_service();
    void stop_snapclient_service();

    void set_metadata_cb(std::function<void(Song const&)>);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif //KITCHENSOUND_SNAPCAST_CONTROLLER_H
