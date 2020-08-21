#ifndef KITCHENSOUND_MODEL_H
#define KITCHENSOUND_MODEL_H

#include <string>
#include <vector>

enum NETWORK_STATE {
    WIFI,
    BLUETOOTH
};

enum PAGES {
    INACTIVE,
    LOADING,
    STREAM_BROWSING,
    STREAM_PLAYING,
    BT_PLAYING
};

struct RadioStationStream {
    std::string name;
    std::string url;
    std::string image_url;
};

struct BasePageModel {
    int hour;
    int minutes;
};

struct VolumeModel {
    int active_change_timeout;
};

struct InactivePageModel {
    int remaining_amp_cooldown_time;
};

struct LoadingPageModel {
    std::string msg;
};

struct StationBrowsingPageModel {
    int offset;
    int selected;
    int confirmed_selection;
    int limit;

    bool times_out;
    int remaining_time;

    std::vector<RadioStationStream> stations;
};

struct StationPlayingPageModel {
    StationPlayingPageModel() : station{},
        station_changed{true}, meta_changed{true}, meta_text{} {};

    RadioStationStream station;

    bool station_changed;

    bool meta_changed;
    std::string meta_text;
};

struct BluetoothPlayingPageModel {
    BluetoothPlayingPageModel() : status_changed{true},
    status{}, meta_changed{true}, meta{} {};

    bool status_changed;
    std::string status;

    bool meta_changed;
    std::string meta;
};

#endif //KITCHENSOUND_MODEL_H
