#include "kitchensound/pages/station_playing_page.h"

#include "kitchensound/mpd_controller.h"
#include "kitchensound/render_text.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"

#define RADIO_IMAGE "img/radio.png"

StationPlayingPage::StationPlayingPage(StateController &ctrl, ResourceManager &res,
                                       Volume &vol, RadioStationStream* initial_station) :
        VolumePage(STREAM_PLAYING, ctrl, vol),
        _res{res}, _model{},
        _text_meta{std::make_unique<RenderText>()},
        _text_status{std::make_unique<RenderText>()} {

    MPDController::init([&](auto new_meta) {
        set_meta_text(new_meta);
    });

    if(initial_station == nullptr)
        throw  std::runtime_error{"StationPlayingPage::C-Tor(): initial radio station is null!"};
    _model.station.name = initial_station->name;
    _model.station.image_url = initial_station->image_url;
    _model.station.url = initial_station->url;
};

StationPlayingPage::~StationPlayingPage() = default;

void StationPlayingPage::set_meta_text(const std::string &new_meta)  {
    if(new_meta != _text_meta->get_current_text()){
        _model.meta_changed = true;
        _model.meta_text = std::string{new_meta};
    }
}

void StationPlayingPage::handle_enter_key() {
    _state.trigger_transition(_page, STREAM_SELECTION);
}

void StationPlayingPage::render(Renderer& renderer) {
    this->render_time(renderer);

    //1. render the radio station artwork if present, otherwise the default
    auto img_ptr = _res.get_cached(_model.station.image_url);
    if (img_ptr == nullptr)
        img_ptr = _res.get_static(RADIO_IMAGE);

    renderer.render_image(img_ptr, 96, 36, 128, 128);

    //2. render the station name
    if (_model.station_changed) {
        _model.station_changed = false;
        _text_status->change_text(renderer, _model.station.name, 160, 180);
    }

    _text_status->update_and_render(renderer);

    //3. render the meta data
    if (_model.meta_changed) {
        _model.meta_changed = false;
        _text_meta->change_text(renderer, _model.meta_text, 160, 210);
    }

    _text_meta->update_and_render(renderer);

    this->render_volume(renderer);
}

void StationPlayingPage::set_station_playing(RadioStationStream* stream) {
    if (_model.station.name != stream->name) {
        _model.station_changed = true;
        _model.station.name = std::string{stream->name};
        _model.station.url = std::string{stream->url};
        _model.station.image_url = std::string{stream->image_url};
        MPDController::get().stop_playback();
        MPDController::get().playback_stream(_model.station.url);
    }
}

void StationPlayingPage::enter_page(PAGES origin, void* payload) {
    BasePage::update_time();
    VolumePage::enter_page(origin, payload);
    if (origin != STREAM_SELECTION) {
        MPDController::get().stop_playback();
        MPDController::get().playback_stream(_model.station.url);
    }else{
        if(payload == nullptr)
            throw std::runtime_error{"StationPlayingPage::enter_page(): Called from STREAM_SELECTION without passing a stream!"};
        set_station_playing(reinterpret_cast<RadioStationStream*>(payload));
    }
}

void* StationPlayingPage::leave_page(PAGES destination) {
    if (destination != STREAM_SELECTION) {
        MPDController::get().stop_playback();
    }
    return nullptr;
}