#include "kitchensound/config.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

Configuration::Configuration(const std::filesystem::path& file) {
    conf.readFile(file.c_str());
}

Configuration::~Configuration() {
    SPDLOG_DEBUG("Configuration dropped.");
}

std::string Configuration::get_input_device(INPUT_SOURCES source) {
    std::string device;
    switch (source) {
        case INPUT_SOURCES::WHEEL_AXIS:
            device =  conf.lookup("input.wheel_axis").c_str();
            break;
        case INPUT_SOURCES::ENTER_KEY:
            device = conf.lookup("input.enter_key").c_str();
            break;
        case INPUT_SOURCES::MENU_KEY:
            device = conf.lookup("input.menu_key").c_str();
            break;
        case INPUT_SOURCES::POWER_KEY:
            device = conf.lookup("input.power_key").c_str();
            break;
        default:
            throw std::runtime_error("Found unsupported INPUT_SOURCE!");
    };
    SPDLOG_INFO("Read input device for source -> {0}; {1}", source, device);
    return device;
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

        SPDLOG_INFO("Read radio station -> {0}; {1}; {2};", stream.name, stream.url, stream.image_url);
        entries.push_back(std::move(stream));
    }

    return entries;
}

long Configuration::get_default_volume() {
    int v;
    conf.lookupValue("default_volume", v);
    SPDLOG_INFO("Read default volume -> {0}", v);
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

    SPDLOG_INFO("Read display standby config -> {0}; {1}:{2} - {3}:{4};",
                s.enabled, s.start_hour, s.start_minute, s.end_hour, s.end_minute);
    return s;
}

Configuration::MPDConfig Configuration::read_mpd_config_from(libconfig::Setting& mpdconf) {
    MPDConfig s{};
    if(!mpdconf.exists("address") || !mpdconf.exists("port")){
        throw std::runtime_error{"Either mpd.address or mpd.port is missing in the configuration!"};
    }
    s.address = mpdconf.lookup("address").c_str();
    s.port    = mpdconf.lookup("port");

    SPDLOG_INFO("Read mpd configuration -> {0}:{1}", s.address, s.port);

    return s;
}

Configuration::MPDConfig Configuration::get_mpd_config() {
    auto& mpdconf = conf.lookup("mpd");
    return read_mpd_config_from(mpdconf);
}

Configuration::SnapcastConfig Configuration::get_snapcast_config() {
    auto& c = conf.lookup("snapcast");
    SnapcastConfig s{};
    if(!c.exists("bin") || !c.exists("host") || !c.exists("port") || !c.exists("has_mpd_feed"))
        throw std::runtime_error{"Missing Snapcast Configuration value. One or multiple of: bin, host, port, has_mpd_feed"};

    s.bin           = c.lookup("bin").c_str();
    s.host          = c.lookup("host").c_str();
    s.port          = c.lookup("port");
    s.has_mpd_feed  = c.lookup("has_mpd_feed");

    if(s.has_mpd_feed){
        s.mpd_feed = read_mpd_config_from(c.lookup("mpd_feed"));
    }

    //mirror the alsa pcm device name to the snapcast config
    s.alsa_pcm = get_alsa_device_name(ALSA_DEVICES::PCM_DEVICE);

    SPDLOG_INFO("Read snapcast configuration -> {}; {}:{}; {}; {}:{}", s.bin, s.host, s.port, s.has_mpd_feed, s.mpd_feed.address, s.mpd_feed.port);

    return s;
}

Configuration::AnalyticsConfig Configuration::get_analytics_config() {
    auto& c = conf.lookup("analytics");
    AnalyticsConfig s{};
    if(!c.exists("enabled") || c.lookup("enabled") && !c.exists("destination_host")){
        throw std::runtime_error{"Missing analytics configuration: either enabled = false or with destination_host"};
    }

    s.enabled = c.lookup("enabled");
    s.dest_host = c.lookup("destination_host").c_str();

    SPDLOG_INFO("Read analytics configuration -> {}; {}", s.enabled, s.dest_host);

    return s;
}

Configuration::SongFaverConfig Configuration::get_songfaver_config() {
    auto& c = conf.lookup("song_faving");
    SongFaverConfig s{};
    if(!c.exists("enabled") || c.lookup("enabled") && !c.exists("destination_host")){
        throw std::runtime_error{"Missing song_faving configuration: either enabled = false or with destination_host"};
    }

    s.enabled = c.lookup("enabled");
    s.dest_host = c.lookup("destination_host").c_str();

    SPDLOG_INFO("Read songfaver configuration -> {}; {}", s.enabled, s.dest_host);

    return s;
}

int Configuration::get_gpio_pin(GPIO_PIN request_pin) {
    std::string path;
    int def;
    switch (request_pin) {
        case GPIO_PIN::DISPLAY_BACKLIGHT:
            path = "gpio.display_backlight";
            def = 13;
            break;
        case GPIO_PIN::AMPLIFIER_POWER:
            path = "gpio.amplifier_power";
            def = 4;
            break;
        default:
            throw std::runtime_error{"Requested unknown GPIO Pin!"};
    }

    int pin;
    if(!conf.lookupValue(path, pin)){
        pin = def;
        SPDLOG_WARN("Failed to read requested pin, defaulting -> {0}; {1}", request_pin, def);
    }else
        SPDLOG_INFO("Read pin for gpio -> {0}; {1}", pin, request_pin);

    return pin;
}

std::string Configuration::get_alsa_device_name(ALSA_DEVICES requested_device) {
    std::string path;
    std::string def;
    switch(requested_device) {
        case ALSA_DEVICES::PCM_DEVICE:
            path = "alsa.pcm_device";
            def = "volumedev";
            break;

        case ALSA_DEVICES::MIXER_CONTROL:
            path = "alsa.mixer_control";
            def = "Master";
            break;

        case ALSA_DEVICES::MIXER_CARD:
            path = "alsa.mixer_card";
            def = "default";
            break;
    }

    std::string device = conf.lookup(path);
    if(device.empty()){
        device = def;
        SPDLOG_WARN("Failed to read alsa device name, defaulting -> {0}; {1}", requested_device, def);
    } else
        SPDLOG_INFO("Read alsa device name -> {0}; {1}", requested_device, device);

    return device;
}

std::filesystem::path Configuration::get_folder(std::string const& conf_path, std::string const& def) {
    std::string tmp = conf.lookup(conf_path);
    std::filesystem::path folder;
    if(tmp.empty()){
        folder = std::filesystem::path{def};
        SPDLOG_WARN("Failed to read, defaulting -> {0}; {1}", conf_path, folder.string());
    }else {
        folder = std::filesystem::path{tmp};
        SPDLOG_INFO("Read path -> {0}; {1}", conf_path, folder.string());
    }
    return folder;
}

std::filesystem::path Configuration::get_res_folder() {
    return get_folder("resource_root", "../res");
}

std::filesystem::path Configuration::get_cache_folder() {
    return get_folder("cache_root", "../cache");
}

MAKE_ENUM_STRINGIFY(ENUM_INPUT_SOURCES, Configuration::INPUT_SOURCES)
MAKE_ENUM_STRINGIFY(ENUM_GPIO_PIN, Configuration::GPIO_PIN)
MAKE_ENUM_STRINGIFY(ENUM_ALSA_DEVICES, Configuration::ALSA_DEVICES)