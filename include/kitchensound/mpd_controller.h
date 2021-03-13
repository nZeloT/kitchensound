#ifndef KITCHENSOUND_MPD_CONTROLLER_H
#define KITCHENSOUND_MPD_CONTROLLER_H

#include <string>
#include <memory>
#include <functional>

#include "kitchensound/config.h"


class MPDController {
public:
    explicit MPDController(Configuration::MPDConfig);
    ~MPDController();

    void playback_stream(const std::string& stream_url);

    void stop_playback();

    void force_metadata_update();

    void update(long ms_delta_time);

    void set_metadata_callback(std::function<void(const std::string&)>);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif //KITCHENSOUND_MPD_CONTROLLER_H
