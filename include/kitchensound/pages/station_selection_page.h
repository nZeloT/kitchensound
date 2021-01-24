#ifndef KITCHENSOUND_STATION_SELECTION_PAGE_H
#define KITCHENSOUND_STATION_SELECTION_PAGE_H

#include <utility>

#include "kitchensound/pages/selection_page.h"
#include "kitchensound/mpd_controller.h"

class StationSelectionPage : public SelectionPage<RadioStationStream> {
public:
    StationSelectionPage(StateController* ctrl, ResourceManager& res, std::vector<RadioStationStream> streams);

    void enter_page(PAGES origin) override {
        this->update_time();
        _model.times_out = false;
        _model.remaining_time = -1;
        if(origin == STREAM_PLAYING)
            activate_timeout();
    }

    void leave_page(PAGES destination) override {
        if(destination != STREAM_PLAYING && _model.times_out) {
            //model only times out if browsing page was called from stream playing
            //but the new destination isn't playing; this requires halting the mpd playback
            MPDController::get().stop_playback();
        }
    };

    void handle_wheel_input(int delta) override;
    void handle_enter_key() override;
    void render(Renderer &renderer) override;

    RadioStationStream get_selected_stream() { return _sp_model.data[_model.confirmed_selection]; };

private:
    void activate_timeout();

    StationBrowsingPageModel _model;
};

#endif //KITCHENSOUND_STATION_SELECTION_PAGE_H
