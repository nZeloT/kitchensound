#include "kitchensound/pages/page_loader.h"

#include "kitchensound/config.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/init_system_conns.h"

#include "kitchensound/pages/inactive_page.h"
#include "kitchensound/pages/loading_page.h"
#include "kitchensound/pages/menu_selection_page.h"
#include "kitchensound/pages/station_playing_page.h"
#include "kitchensound/pages/station_selection_page.h"
#include "kitchensound/pages/bt_playing_page.h"
#include "kitchensound/pages/options_page.h"

std::unordered_map<PAGES, std::unique_ptr<BasePage>> load_pages(
        Configuration &conf,
        StateController &ctrl,
        TimerManager& tm,
        ResourceManager &res) {

    std::unordered_map<PAGES, std::unique_ptr<BasePage>> pages;

    auto gpio = init_gpio(conf);

    auto standby = init_standby(conf);

    auto volume = init_volume(conf);

    auto playback = init_playback(conf);

    auto bt_controller = init_bt_controller(playback);

    auto mpd_controller = init_mpd_controller(conf);

    auto os_util = init_os_util();

    pages.emplace(INACTIVE, std::make_unique<InactivePage>(ctrl, tm, standby, gpio));
    pages.emplace(LOADING, std::make_unique<LoadingPage>(ctrl, tm));
    pages.emplace(MENU_SELECTION, std::make_unique<MenuSelectionPage>(ctrl, tm, res));
    auto stream_sel = std::make_unique<StationSelectionPage>(ctrl, tm, res, mpd_controller, conf.get_radio_stations());
    auto station = stream_sel->get_selected_stream();
    pages.emplace(STREAM_SELECTION, std::move(stream_sel));
    pages.emplace(STREAM_PLAYING, std::make_unique<StationPlayingPage>(ctrl, tm, res, volume, mpd_controller, station));
    pages.emplace(BT_PLAYING, std::make_unique<BluetoothPlayingPage>(ctrl, tm, res, volume, bt_controller));
    pages.emplace(OPTIONS, std::make_unique<OptionsPage>(ctrl, tm, os_util));

    return pages;
}