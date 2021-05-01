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

    auto bt_controller = init_bt_controller(bb.fdreg, bb.analytics, playback);

    auto mpd_controller = init_mpd_controller(bb.fdreg, bb.analytics, bb.conf);

    auto os_util = init_os_util();

    auto song_faver = init_song_faver(bb.net, bb.conf);

    pages.emplace(PAGES::INACTIVE, std::make_unique<InactivePage>(bb, standby, gpio));
    pages.emplace(PAGES::LOADING, std::make_unique<LoadingPage>(bb));
    pages.emplace(PAGES::MENU_SELECTION, std::make_unique<MenuSelectionPage>(bb));
    auto stream_sel = std::make_unique<StationSelectionPage>(bb, mpd_controller, bb.conf->get_radio_stations());
    auto station = stream_sel->get_selected_stream();
    pages.emplace(PAGES::STREAM_SELECTION, std::move(stream_sel));
    pages.emplace(PAGES::STREAM_PLAYING, std::make_unique<StationPlayingPage>(bb, volume, song_faver, mpd_controller, station));
    pages.emplace(PAGES::BT_PLAYING, std::make_unique<BluetoothPlayingPage>(bb, volume, song_faver, bt_controller));
    pages.emplace(PAGES::OPTIONS, std::make_unique<OptionsPage>(bb, os_util, song_faver));

    return pages;
}