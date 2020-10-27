#include "kitchensound/config.h"

Configuration::Configuration(const std::string &file) {
    conf.readFile(file.c_str());
}

Configuration::~Configuration() = default;

std::string Configuration::get_input_device(INPUT_SOURCES source) {
    switch (source) {
        case WHEEL_AXIS:
            return (const char*)conf.lookup("input.wheel_axis");
        case ENTER_KEY:
            return (const char*)conf.lookup("input.enter_key");
        case NETWORK_KEY:
            return (const char*)conf.lookup("input.network_key");
        case LED_KEY:
            return (const char*)conf.lookup("input.led_key");
        default:
            throw std::runtime_error("Found unsupported INPUT_SOURCE!");
    };
}

std::vector<RadioStationStream> Configuration::get_radio_stations() {
    auto& streams = conf.lookup("streams");
    int count = streams.getLength();
    std::vector<RadioStationStream> entries{};

    for(int i = 0; i < count; i++) {
        auto& s = streams[i];
        RadioStationStream stream{};
        stream.name = (const char*)s.lookup("name");
        stream.url  = (const char*)s.lookup("url");
        if(s.exists("image_url"))
            stream.image_url = (const char*)s.lookup("image_url");
        entries.push_back(std::move(stream));
    }

    return entries;
}

long Configuration::get_volume() {
    int v;
    conf.lookupValue("volume", v);
    return static_cast<long>(v);
}
