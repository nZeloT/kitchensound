#include "kitchensound/pages/station_selection_page.h"

#include <spdlog/spdlog.h>


#include "kitchensound/timeouts.h"
#include "kitchensound/input_event.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/mpd_controller.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/timer.h"

#define RADIO_IMAGE "img/radio.png"

StationSelectionPage::StationSelectionPage(StateController& ctrl, TimerManager& tm, ResourceManager& res,
                                           std::shared_ptr<MPDController>& mpd,
                                           std::vector<RadioStationStream> streams) :
        SelectionPage<RadioStationStream>(STREAM_SELECTION, ctrl, tm, res, std::move(streams)),
                _mpd{mpd}, _model{}, _auto_leave_timer{std::make_unique<Timer>(AUTO_LEAVE_BROWSING, false, [this](){
                    this->_state.trigger_transition(this->_page, STREAM_PLAYING);
                })} {
    load_images();
};

StationSelectionPage::~StationSelectionPage() = default;

void StationSelectionPage::enter_page(PAGES origin, void* payload)  {
    this->update_time();
    _model.times_out = false;
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
    _auto_leave_timer->reset();
    SPDLOG_INFO("Activated timeout.");
}

void StationSelectionPage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT) {
        _state.trigger_transition(_page, STREAM_PLAYING);
        _model.confirmed_selection = _sp_model.selected;
        SPDLOG_INFO("Transitioning with new stream -> {0}", _model.confirmed_selection);
    }
}

void StationSelectionPage::handle_wheel_input(int delta) {
    SelectionPage<RadioStationStream>::handle_wheel_input(delta);
    if (_model.times_out)
        _auto_leave_timer->reset();
}

void StationSelectionPage::update(long ms_delta_update) {
    if(_model.times_out)
        _auto_leave_timer->update(ms_delta_update);
    BasePage::update(ms_delta_time);
}