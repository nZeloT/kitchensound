#include "kitchensound/pages/station_playing_page.h"

#include "kitchensound/state_controller.h"

#define RADIO_IMAGE "img/radio.png"

void StationPlayingPage::handle_enter_key() {
    _state->trigger_transition(_page, STREAM_SELECTION);
}

void StationPlayingPage::render(Renderer &renderer) {
    this->render_time(renderer);

    //1. render the radio station artwork if present, otherwise the default
    SDL_Rect dstrect{96, 36, 128, 128};
    auto img_ptr = _res.get_cached(_model.station.image_url);
    if (img_ptr == nullptr)
        img_ptr = _res.get_static(RADIO_IMAGE);

    renderer.render_image(reinterpret_cast<SDL_Surface *>(img_ptr), dstrect);

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

void StationPlayingPage::set_station_playing(RadioStationStream &stream) {
    if (_model.station.name != stream.name) {
        _model.station_changed = true;
        _model.station.name = std::string{stream.name};
        _model.station.url = std::string{stream.url};
        _model.station.image_url = std::string{stream.image_url};
        MPDController::get().stop_playback();
        MPDController::get().playback_stream(_model.station.url);
    }
}

void StationPlayingPage::enter_page(PAGES origin)  {
    BasePage::update_time();
    VolumePage::enter_page(origin);
    if(origin != STREAM_SELECTION) {
        reset_model();
    }
}

void StationPlayingPage::leave_page(PAGES destination) {
    if(destination != STREAM_SELECTION) {
        MPDController::get().stop_playback();
        reset_model();
    }
}

void StationPlayingPage::reset_model() {
    _model.meta_changed = false;
    _model.meta_text = "META TEXT";
    _model.station_changed = false;
    _model.station.name = "RADIO STATION";
    _model.station.url = "STREAM";
    _model.station.image_url = "IMAGE";
}

