#include "kitchensound/pages/station_playing_page.h"

#include "kitchensound/input_event.h"
#include "kitchensound/mpd_controller.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"

#define RADIO_IMAGE "img/radio.png"

StationPlayingPage::StationPlayingPage(StateController &ctrl, ResourceManager &res, std::shared_ptr<Volume> &vol,
                                       std::shared_ptr<MPDController> &mpd, RadioStationStream *initial_station) :
        PlayingPage(STREAM_PLAYING, ctrl, res, vol),
        _mpd{mpd}, _model{} {

    _mpd->set_metadata_callback([&](auto new_meta) {
        set_metadata_text(new_meta);
    });

    if (initial_station == nullptr)
        throw std::runtime_error{"StationPlayingPage::C-Tor(): initial radio station is null!"};
    _model.station.name = initial_station->name;
    _model.station.image_url = initial_station->image_url;
    _model.station.url = initial_station->url;

    set_source_text(_model.station.name);
    set_image(_model.station.image_url, RADIO_IMAGE);
}

StationPlayingPage::~StationPlayingPage() = default;

void StationPlayingPage::handle_enter_key(InputEvent& inev) {
    PlayingPage::handle_enter_key(inev);
    if(inev.value == INEV_KEY_SHORT)
        _state.trigger_transition(_page, STREAM_SELECTION);
}

void StationPlayingPage::set_station_playing(RadioStationStream *stream) {
    if (_model.station.name != stream->name) {
        _model.station.name = std::string{stream->name};
        _model.station.url = std::string{stream->url};
        _model.station.image_url = std::string{stream->image_url};

        set_source_text(_model.station.name);
        set_image(_model.station.image_url, RADIO_IMAGE);

        _mpd->stop_playback();
        _mpd->playback_stream(_model.station.url);
    }else{
        //already playing the stream; possibly some time went by not updating the metadata, so force an update now
        _mpd->force_metadata_update();
    }
}

void StationPlayingPage::enter_page(PAGES origin, void *payload) {
    BasePage::update_time();
    VolumePage::enter_page(origin, payload);
    if (origin != STREAM_SELECTION) {
        _mpd->stop_playback();
        _mpd->playback_stream(_model.station.url);
        set_image(_model.station.image_url, RADIO_IMAGE);
    } else {
        if (payload == nullptr)
            throw std::runtime_error{
                    "StationPlayingPage::enter_page(): Called from STREAM_SELECTION without passing a stream!"};
        set_station_playing(reinterpret_cast<RadioStationStream *>(payload));
    }
}

void *StationPlayingPage::leave_page(PAGES destination) {
    if (destination != STREAM_SELECTION) {
        _mpd->stop_playback();
    }
    return nullptr;
}