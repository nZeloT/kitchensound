#include "kitchensound/pages/page_loader.h"

#include "kitchensound/config.h"
#include "kitchensound/volume.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/gpio_util.h"

#include "kitchensound/pages/inactive_page.h"
#include "kitchensound/pages/loading_page.h"
#include "kitchensound/pages/menu_selection_page.h"
#include "kitchensound/pages/station_playing_page.h"
#include "kitchensound/pages/station_selection_page.h"
#include "kitchensound/pages/bt_playing_page.h"
#include "kitchensound/pages/options_page.h"

std::unordered_map<PAGES, std::unique_ptr<BasePage>> load_pages(
        Configuration& conf,
        StateController& ctrl,
        ResourceManager& res,
        Volume& vol,
        GpioUtil& gpio) {

    std::unordered_map<PAGES, std::unique_ptr<BasePage>> pages;
    pages.emplace(INACTIVE, std::make_unique<InactivePage>(ctrl, [&gpio](bool new_state){
        if(new_state)
            gpio.turn_on_amplifier();
        else
            gpio.turn_off_amplifier();
    }));
    pages.emplace(LOADING, std::make_unique<LoadingPage>(ctrl));
    pages.emplace(MENU_SELECTION, std::make_unique<MenuSelectionPage>(ctrl, res));
    auto stream_sel = std::make_unique<StationSelectionPage>(ctrl, res, conf.get_radio_stations());
    auto station = stream_sel->get_selected_stream();
    pages.emplace(STREAM_SELECTION, std::move(stream_sel));
    pages.emplace(STREAM_PLAYING, std::make_unique<StationPlayingPage>(ctrl, res, vol, station));
    pages.emplace(BT_PLAYING, std::make_unique<BluetoothPlayingPage>(ctrl, vol, res));
    pages.emplace(OPTIONS, std::make_unique<OptionsPage>(ctrl));

    return pages;
}