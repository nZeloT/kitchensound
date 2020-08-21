#ifndef KITCHENSOUND_CONFIG_H
#define KITCHENSOUND_CONFIG_H

#include <memory>
#include <string>
#include <vector>

#include <libconfig.h++>

#include "kitchensound/model.h"

class Configuration {
public:
    enum INPUT_SOURCES {
        WHEEL_AXIS,
        ENTER_KEY,
        NETWORK_KEY,
        LED_KEY
    };

    explicit Configuration(const std::string& file);
    ~Configuration();

    std::string get_input_device(INPUT_SOURCES source);
    std::vector<RadioStationStream> get_radio_stations();
    long get_volume();

private:
    libconfig::Config conf;
};

#endif //KITCHENSOUND_CONFIG_H
