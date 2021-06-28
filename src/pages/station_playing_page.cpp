#include "kitchensound/pages/station_playing_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/input_event.h"
#include "kitchensound/mpd_controller.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/application_backbone.h"

#define RADIO_IMAGE "img/radio.png"

StationPlayingPage::StationPlayingPage(ApplicationBackbone &bb, std::shared_ptr<Volume> &vol,
                                       std::shared_ptr<SongFaver> &faver, std::shared_ptr<MPDController> &mpd,
                                       RadioStationStream *initial_station) :
        PlayingPage(PAGES::STREAM_PLAYING, bb, vol, faver),
        _mpd{mpd}, _model{} {

    _mpd->set_metadata_callback([&](auto new_song) {
        set_current_song(new_song);
    });

    if (initial_station == nullptr)
        throw std::runtime_error{"StationPlayingPage::C-Tor(): initial radio station is null!"};
    _model.station.name = initial_station->name;
    _model.station.image_url = initial_station->image_url;
    _model.station.url = initial_station->url;

    set_source(_model.station.name);
    set_image(_model.station.image_url, RADIO_IMAGE);
}

StationPlayingPage::~StationPlayingPage() {
    SPDLOG_DEBUG("Dropped Station Playing Page");
}

void StationPlayingPage::handle_enter_key(InputEvent &inev) {
    PlayingPage::handle_enter_key(inev);
    if (inev.value == INEV_KEY_SHORT)
        _bb.ctrl->trigger_transition(_page, PAGES::STREAM_SELECTION);
}

void StationPlayingPage::set_station_playing(RadioStationStream *stream) {
    if (_model.station.name != stream->name) {
        _model.station.name = std::string{stream->name};
        _model.station.url = std::string{stream->url};
        _model.station.image_url = std::string{stream->image_url};

        set_source(_model.station.name);
        set_current_song(EMPTY_SONG);
        set_image(_model.station.image_url, RADIO_IMAGE);

        _mpd->stop_playback();
        _mpd->playback_stream(_model.station.name, _model.station.url);
    } else {
        //already playing the stream; possibly some time went by not updating the metadata, so force an update now
        _mpd->force_metadata_update();
    }
}

void StationPlayingPage::enter_page(PAGES origin, void *payload) {
    PlayingPage::enter_page(origin, payload);
    if (origin != PAGES::STREAM_SELECTION) {
        _mpd->stop_playback();
        _mpd->playback_stream(_model.station.name, _model.station.url);
        set_source(_model.station.name);
        set_current_song(EMPTY_SONG);
        set_image(_model.station.image_url, RADIO_IMAGE);
    } else {
        if (payload == nullptr)
            throw std::runtime_error{
                    "StationPlayingPage::enter_page(): Called from STREAM_SELECTION without passing a stream!"};
        set_station_playing(reinterpret_cast<RadioStationStream *>(payload));
    }
}

void *StationPlayingPage::leave_page(PAGES destination) {
    if (destination != PAGES::STREAM_SELECTION) {
        _mpd->stop_playback();
    }
    PlayingPage::leave_page(destination);
    return nullptr;
}