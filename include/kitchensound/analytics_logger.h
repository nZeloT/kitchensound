#ifndef KITCHENSOUND_ANALYTICS_LOGGER_H
#define KITCHENSOUND_ANALYTICS_LOGGER_H

#include <memory>

#include "kitchensound/pages/pages.h"
#include "kitchensound/config.h"

class NetworkController;

enum class PLAYBACK_SOURCE {RADIO_STREAM, BLUETOOTH, SNAPCAST};

class AnalyticsLogger {

public:
    AnalyticsLogger(std::unique_ptr<NetworkController> &, Configuration::AnalyticsConfig);

    ~AnalyticsLogger();

    void log_page_change(PAGES, PAGES);

    void log_playback_change(PLAYBACK_SOURCE, std::string const &, bool);

    void log_playback_song_change(std::string const &, std::string const & = "",
                                  std::string const& = "", std::string const & = "");

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;

};

#endif //KITCHENSOUND_ANALYTICS_LOGGER_H
