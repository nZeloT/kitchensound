#ifndef KITCHENSOUND_CONFIG_H
#define KITCHENSOUND_CONFIG_H

#include <memory>
#include <vector>
#include <iostream>
#include <filesystem>

#include <libconfig.h++>

#include "kitchensound/enum_helper.h"
#include "kitchensound/radio_station_stream.h"

#define ENUM_INPUT_SOURCES(DO,ACCESSOR) \
    DO(WHEEL_AXIS, ACCESSOR)            \
    DO(ENTER_KEY, ACCESSOR)             \
    DO(MENU_KEY, ACCESSOR)              \
    DO(POWER_KEY, ACCESSOR)             \

#define ENUM_GPIO_PIN(DO,ACCESSOR)      \
    DO(DISPLAY_BACKLIGHT,ACCESSOR)      \
    DO(AMPLIFIER_POWER,ACCESSOR)        \

#define ENUM_ALSA_DEVICES(DO,ACCESSOR)  \
    DO(PCM_DEVICE,ACCESSOR)             \
    DO(MIXER_CONTROL,ACCESSOR)          \
    DO(MIXER_CARD,ACCESSOR)             \

class Configuration {
public:
    enum class INPUT_SOURCES {
        ENUM_INPUT_SOURCES(MAKE_ENUM,)
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

    struct SnapcastConfig {
        std::string bin;
        std::string host;
        int port;
        std::string alsa_pcm;
    };

    struct AnalyticsConfig {
        bool enabled;
        std::string dest_host;
    };

    struct SongFaverConfig {
        bool enabled;
        std::string dest_host;
    };

    enum class GPIO_PIN {
        ENUM_GPIO_PIN(MAKE_ENUM,)
    };

    enum class ALSA_DEVICES {
        ENUM_ALSA_DEVICES(MAKE_ENUM,)
    };

    explicit Configuration(const std::filesystem::path& file);
    ~Configuration();

    std::string get_input_device(INPUT_SOURCES source);
    std::vector<RadioStationStream> get_radio_stations();
    long get_default_volume();
    int get_gpio_pin(GPIO_PIN request_pin);
    std::string get_alsa_device_name(ALSA_DEVICES requested_device);
    DisplayStandbyConfig get_display_standby();
    MPDConfig get_mpd_config();
    SnapcastConfig get_snapcast_config();
    AnalyticsConfig get_analytics_config();
    SongFaverConfig get_songfaver_config();
    std::filesystem::path get_res_folder();
    std::filesystem::path get_cache_folder();

private:
    std::filesystem::path get_folder(std::string const& conf_path, std::string const& def);

    libconfig::Config conf;
};

std::ostream& operator<<(std::ostream&,Configuration::INPUT_SOURCES);
std::ostream& operator<<(std::ostream&,Configuration::GPIO_PIN);
std::ostream& operator<<(std::ostream&,Configuration::ALSA_DEVICES);

#endif //KITCHENSOUND_CONFIG_H
