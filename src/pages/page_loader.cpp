#include "kitchensound/pages/page_loader.h"

#include "kitchensound/config.h"
#include "kitchensound/init_system_conns.h"
#include "kitchensound/application_backbone.h"

#include "kitchensound/pages/inactive_page.h"
#include "kitchensound/pages/loading_page.h"
#include "kitchensound/pages/menu_selection_page.h"
#include "kitchensound/pages/station_playing_page.h"
#include "kitchensound/pages/station_selection_page.h"
#include "kitchensound/pages/bt_playing_page.h"
#include "kitchensound/pages/options_page.h"

std::unordered_map<PAGES, std::unique_ptr<BasePage>> load_pages(ApplicationBackbone& bb) {

    std::unordered_map<PAGES, std::unique_ptr<BasePage>> pages;

    auto gpio = init_gpio(bb.conf);

    auto standby = init_standby(bb.conf, bb.fdreg);

    auto volume = init_volume(bb.conf);

    auto playback = init_playback(bb.conf);

    auto bt_controller = init_bt_controller(playback, bb.fdreg);

    auto mpd_controller = init_mpd_controller(bb.conf, bb.fdreg);

    auto os_util = init_os_util();

    pages.emplace(INACTIVE, std::make_unique<InactivePage>(bb, standby, gpio));
    pages.emplace(LOADING, std::make_unique<LoadingPage>(bb));
    pages.emplace(MENU_SELECTION, std::make_unique<MenuSelectionPage>(bb));
    auto stream_sel = std::make_unique<StationSelectionPage>(bb, mpd_controller, bb.conf->get_radio_stations());
    auto station = stream_sel->get_selected_stream();
    pages.emplace(STREAM_SELECTION, std::move(stream_sel));
    pages.emplace(STREAM_PLAYING, std::make_unique<StationPlayingPage>(bb, volume, mpd_controller, station));
    pages.emplace(BT_PLAYING, std::make_unique<BluetoothPlayingPage>(bb, volume, bt_controller));
    pages.emplace(OPTIONS, std::make_unique<OptionsPage>(bb, os_util));

    return pages;
}