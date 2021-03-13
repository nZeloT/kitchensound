#ifndef KITCHENSOUND_CONFIG_H
#define KITCHENSOUND_CONFIG_H

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include <libconfig.h++>

#include "kitchensound/radio_station_stream.h"

class Configuration {
public:
    enum INPUT_SOURCES {
        WHEEL_AXIS,
        ENTER_KEY,
        MENU_KEY,
        POWER_KEY
    };

    struct DisplayStandbyConfig {
        bool enabled;
        long start_hour;
        long start_minute;
        long end_hour;
        long end_minute;
    };

    struct MPDConfig {
        std::string address;
        int port;
    };

    enum GPIO_PIN {
        DISPLAY_BACKLIGHT,
        AMPLIFIER_POWER
    };

    enum ALSA_DEVICES {
        PCM_DEVICE,
        MIXER_CONTROL,
        MIXER_CARD
    };

    explicit Configuration(const std::string& file);
    ~Configuration();

    std::string get_input_device(INPUT_SOURCES source);
    std::vector<RadioStationStream> get_radio_stations();
    long get_default_volume();
    int get_gpio_pin(GPIO_PIN request_pin);
    std::string get_alsa_device_name(ALSA_DEVICES requested_device);
    DisplayStandbyConfig get_display_standby();
    MPDConfig get_mpd_config();
    std::filesystem::path get_res_folder();
    std::filesystem::path get_cache_folder();

private:
    std::filesystem::path get_folder(std::string const& conf_path, std::string const& def);

    libconfig::Config conf;
};

#endif //KITCHENSOUND_CONFIG_H
