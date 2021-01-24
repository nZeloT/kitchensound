#ifndef KITCHENSOUND_MODEL_H
#define KITCHENSOUND_MODEL_H

#include <string>
#include <vector>
#include <ctime>

enum PAGES {
    INACTIVE,
    LOADING,
    STREAM_SELECTION,
    STREAM_PLAYING,
    BT_PLAYING,
    OPTIONS,
    MENU_SELECTION
};

struct RadioStationStream {
    std::string name;
    std::string url;
    std::string image_url;
};

struct BasePageModel {
    std::time_t current_time = 0;
    int hour = 0;
    int minute = 0;
};

struct VolumeModel {
    int active_change_timeout;
};

struct InactivePageModel {
    std::time_t amp_cooldown_start;
    PAGES last_seen;
};

struct LoadingPageModel {
    std::string msg;
};

template<class T>
struct SelectionPageModel {
    int offset;
    int selected;
    int limit;

    std::vector<T> data;
};

struct ModeModel {
    std::string name;
    PAGES ref_page;
    std::string static_image;
};

struct StationBrowsingPageModel {
    int confirmed_selection;

    bool times_out;
    int remaining_time;
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

const std::vector<ModeModel> MENUS = {
        {"Radio", STREAM_PLAYING, "img/radio.png"},
        {"Bluetooth", BT_PLAYING, "img/bluetooth.png"},
        {"Optionen", OPTIONS, "img/gears.png"}
};

#endif //KITCHENSOUND_MODEL_H
