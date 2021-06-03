#ifndef KITCHENSOUND_MPD_CONTROLLER_H
#define KITCHENSOUND_MPD_CONTROLLER_H

#include <string>
#include <memory>
#include <functional>

#include "kitchensound/enum_helper.h"
#include "kitchensound/config.h"

class FdRegistry;
class AnalyticsLogger;
struct Song;

enum MPD_TAG {
    RAW    = 1,
    TITLE  = 2,
    ARTIST = 4,
    ALBUM  = 8
};
typedef int MPD_TAGS;

class MPDController {
public:
    MPDController(std::unique_ptr<FdRegistry>&, std::unique_ptr<AnalyticsLogger>&, Configuration::MPDConfig);
    ~MPDController();

    void playback_stream(const std::string &, const std::string&);

    void stop_playback();

    void force_metadata_update();

    void set_metadata_callback(std::function<void(const Song&)>);

protected:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

class ExtendedMPDController : public MPDController {
public:
    ExtendedMPDController(std::unique_ptr<FdRegistry>&, std::unique_ptr<AnalyticsLogger>&, Configuration::MPDConfig);
    ~ExtendedMPDController();

    void set_tags_to_read(MPD_TAGS tags);

    void start_polling();

    void stop_polling();
};

#endif //KITCHENSOUND_MPD_CONTROLLER_H
