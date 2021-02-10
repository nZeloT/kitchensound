#include "kitchensound/config.h"

#include <spdlog/spdlog.h>

Configuration::Configuration(const std::string &file) {
    conf.readFile(file.c_str());
}

Configuration::~Configuration() = default;

std::string Configuration::get_input_device(INPUT_SOURCES source) {
    switch (source) {
        case WHEEL_AXIS:
            return (const char *) conf.lookup("input.wheel_axis");
        case ENTER_KEY:
            return (const char *) conf.lookup("input.enter_key");
        case MENU_KEY:
            return (const char *) conf.lookup("input.menu_key");
        case POWER_KEY:
            return (const char *) conf.lookup("input.power_key");
        default:
            throw std::runtime_error("Found unsupported INPUT_SOURCE!");
    };
}

std::vector<RadioStationStream> Configuration::get_radio_stations() {
    auto &streams = conf.lookup("streams");
    int count = streams.getLength();
    std::vector<RadioStationStream> entries{};

    for (int i = 0; i < count; i++) {
        auto &s = streams[i];
        RadioStationStream stream{};
        stream.name = (const char *) s.lookup("name");
        stream.url = (const char *) s.lookup("url");
        if (s.exists("image_url"))
            stream.image_url = (const char *) s.lookup("image_url");

        spdlog::info("Configuration::get_radio_stations(): Read radio station {0} with url {1} and image {2}", stream.name, stream.url, stream.image_url);
        entries.push_back(std::move(stream));
    }

    return entries;
}

long Configuration::get_default_volume() {
    int v;
    conf.lookupValue("default_volume", v);
    spdlog::info("Configuration::get_default:volume(): Read default volume of {0}", v);
    return static_cast<long>(v);
}

Configuration::DisplayStandbyConfig Configuration::get_display_standby() {
    auto &standby = conf.lookup("display_standby");
    DisplayStandbyConfig s{};
    int start_hour;
    int start_minute;
    int end_hour;
    int end_minute;
    bool on;
    if (standby.lookupValue("enabled", on)
        && standby.lookupValue("start_hour", start_hour)
        && standby.lookupValue("start_min", start_minute)
        && standby.lookupValue("end_hour", end_hour)
        && standby.lookupValue("end_min", end_minute)) {
        s.enabled = on;
        s.start_minute = start_minute;
        s.start_hour = start_hour;
        s.end_minute = end_minute;
        s.end_hour = end_hour;
    } else
        throw std::runtime_error("Failed to load the display standby times");
    return s;
}

int Configuration::get_gpio_pin(GPIO_PIN request_pin) {
    std::string path;
    int def;
    switch (request_pin) {
        case DISPLAY_BACKLIGHT:
            path = "gpio.display_backlight";
            def = 13;
            break;
        case AMPLIFIER_POWER:
            path = "gpio.amplifier_power";
            def = 4;
            break;
        default:
            throw std::runtime_error{"Requested unknown GPIO Pin!"};
    }

    int pin;
    if(!conf.lookupValue(path, pin)){
        pin = def;
        spdlog::warn("Configuration::get_gpio_pin(): Failed to fetch requested pin {0}, defaulting to {1}", request_pin, def);
    }else
        spdlog::info("Configuration::get_gpio_pin(): Fetched pin {0} for gpio {1}", pin, request_pin);

    return pin;
}

std::string Configuration::get_alsa_device_name(ALSA_DEVICES requested_device) {
    spdlog::info("Configuration::get_alsa_device_name(): Requesting alsa device name for key {0}", requested_device);
    std::string path;
    std::string def;
    switch(requested_device) {
        case PCM_DEVICE:
            path = "alsa.pcm_device";
            def = "volumedev";
            break;

        case MIXER_CONTROL:
            path = "alsa.mixer_control";
            def = "Master";
            break;

        case MIXER_CARD:
            path = "alsa.mixer_card";
            def = "default";
            break;
    }

    std::string device = conf.lookup(path);
    if(device.empty()){
        device = def;
        spdlog::warn("Configuration::get_alsa_device_name(): Failed to fetch requested alsa device {0}, defaulting to {1}", requested_device, def);
    } else
        spdlog::info("Configuration::get_alsa_device_name(): Read {0} for requested alsa device {1}", device, requested_device);

    return device;
}

std::filesystem::path Configuration::get_folder(std::string const& conf_path, std::string const& def) {
    spdlog::info("Configuration::get_folder(): Requesting value for key {0}", conf_path);
    std::string tmp = conf.lookup(conf_path);
    std::filesystem::path folder;
    if(tmp.empty()){
        folder = std::filesystem::path{def};
        spdlog::warn("Configuration::get_folder(): Failed to fetch {0}. Defaulting to `{1}`", conf_path, folder.string());
    }else {
        folder = std::filesystem::path{tmp};
        spdlog::info("Configuration::get_res_folder(): Read {0} as `{1}`", conf_path, folder.string());
    }
    return folder;
}

std::filesystem::path Configuration::get_res_folder() {
    return get_folder("resource_root", "../res");
}

std::filesystem::path Configuration::get_cache_folder() {
    return get_folder("cache_root", "../cache");
}