#include "kitchensound/pages/station_selection_page.h"

#include <spdlog/spdlog.h>


#include "kitchensound/timeouts.h"
#include "kitchensound/input_event.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/mpd_controller.h"
#include "kitchensound/state_controller.h"

#define RADIO_IMAGE "img/radio.png"

StationSelectionPage::StationSelectionPage(StateController& ctrl, ResourceManager& res,
                                           std::shared_ptr<MPDController>& mpd,
                                           std::vector<RadioStationStream> streams) :
        SelectionPage<RadioStationStream>(STREAM_SELECTION, ctrl, res, std::move(streams)), _mpd{mpd},
                                          _model{} {
    load_images();
};

StationSelectionPage::~StationSelectionPage() = default;

void StationSelectionPage::enter_page(PAGES origin, void* payload)  {
    this->update_time();
    _model.times_out = false;
    _model.remaining_time = -1;
    if(origin == STREAM_PLAYING)
        activate_timeout();
    load_images();
}

void* StationSelectionPage::leave_page(PAGES destination)  {
    if(destination != STREAM_PLAYING && _model.times_out) {
        //model only times out if browsing page was called from stream playing
        //but the new destination isn't playing; this requires halting the mpd playback
        _mpd->stop_playback();
    }
    if(destination == STREAM_PLAYING)
        return get_selected_stream();
    else
        return nullptr;
};

RadioStationStream* StationSelectionPage::get_selected_stream() {
    return & _sp_model.data[_model.confirmed_selection];
}

std::string StationSelectionPage::get_text(const RadioStationStream &s) {
    return s.name;
}

void StationSelectionPage::get_image(const RadioStationStream &s, void ** img_data_ptr) {
    _res.get_cached(s.image_url, [this, img_data_ptr](auto s, void* img){
        if(img == nullptr)
            *img_data_ptr = _res.get_static(RADIO_IMAGE);
        else
            *img_data_ptr = img;
    });
}

void StationSelectionPage::activate_timeout() {
    _model.times_out = true;
    _model.remaining_time = BROWSING_TIMEOUT;
    spdlog::info("StationSelectionPage::activate_timeout(): active");
}

void StationSelectionPage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT) {
        _state.trigger_transition(_page, STREAM_PLAYING);
        _model.confirmed_selection = _sp_model.selected;
        spdlog::info("StationSelectionPage::handle_enter_key(): Stream {0} selected; transitioning",
                     _model.confirmed_selection);
    }
}

void StationSelectionPage::handle_wheel_input(int delta) {
    SelectionPage<RadioStationStream>::handle_wheel_input(delta);
    if (_model.times_out)
        _model.remaining_time = BROWSING_TIMEOUT;
}

void StationSelectionPage::render(Renderer& renderer) {
    if (_model.times_out && _model.remaining_time > 0) {
        --_model.remaining_time;
    }
    if (_model.times_out && _model.remaining_time == 0) {
        _state.trigger_transition(_page, STREAM_PLAYING);
    }

    SelectionPage<RadioStationStream>::render(renderer);
}