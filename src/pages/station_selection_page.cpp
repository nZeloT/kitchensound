#include "kitchensound/pages/station_selection_page.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>


#include "kitchensound/timeouts.h"
#include "kitchensound/input_event.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/mpd_controller.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/timer.h"
#include "kitchensound/application_backbone.h"

#define RADIO_IMAGE "img/radio.png"

StationSelectionPage::StationSelectionPage(ApplicationBackbone& bb,
                                           std::shared_ptr<MPDController>& mpd,
                                           std::vector<RadioStationStream> streams) :
        SelectionPage<RadioStationStream>(PAGES::STREAM_SELECTION, bb, std::move(streams)),
                _mpd{mpd}, _model{}, _auto_leave_timer{std::make_unique<Timer>(bb.fdreg, "Station Selection Page Auto Leave", AUTO_LEAVE_BROWSING, false, [this](){
                    this->_bb.ctrl->trigger_transition(this->_page, PAGES::STREAM_PLAYING);
                })} {
    load_images();
};

StationSelectionPage::~StationSelectionPage() = default;

void StationSelectionPage::enter_page(PAGES origin, void* payload)  {
    SelectionPage<RadioStationStream>::enter_page(origin, payload);
    _model.times_out = false;
    if(origin == PAGES::STREAM_PLAYING)
        activate_timeout();
    load_images();
}

void* StationSelectionPage::leave_page(PAGES destination)  {
    _auto_leave_timer->stop();
    if(destination != PAGES::STREAM_PLAYING && _model.times_out) {
        //model only times out if browsing page was called from stream playing
        //but the new destination isn't playing; this requires halting the mpd playback
        _mpd->stop_playback();
    }
    SelectionPage<RadioStationStream>::leave_page(destination);
    if(destination == PAGES::STREAM_PLAYING)
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
    _bb.res->get_cached(s.image_url, [this, img_data_ptr](auto s, void* img){
        SPDLOG_DEBUG("Receiving Cache Image Data -> {}", img == nullptr);
        if(img == nullptr)
            *img_data_ptr = _bb.res->get_static(RADIO_IMAGE);
        else
            *img_data_ptr = img;
    });
}

void StationSelectionPage::activate_timeout() {
    _model.times_out = true;
    _auto_leave_timer->reset_timer();
    SPDLOG_INFO("Activated timeout.");
}

void StationSelectionPage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT) {
        _bb.ctrl->trigger_transition(_page, PAGES::STREAM_PLAYING);
        _model.confirmed_selection = _sp_model.selected;
        SPDLOG_INFO("Transitioning with new stream -> {0}", _model.confirmed_selection);
    }
}

void StationSelectionPage::handle_wheel_input(int delta) {
    SelectionPage<RadioStationStream>::handle_wheel_input(delta);
    if (_model.times_out)
        _auto_leave_timer->reset_timer();
}