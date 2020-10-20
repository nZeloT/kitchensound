#include "kitchensound/pages/station_browsing_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/state_controller.h"

#define RADIO_IMAGE "img/radio.png"
#define ARROW_RIGHT "img/arrow_right.png"
#define ARROW_LEFT  "img/arrow_left.png"

void StationBrowsingPage::activate_timeout()  {
    _model.times_out = true;
    _model.remaining_time = BROWSING_TIMEOUT;
    spdlog::info("StationBrowsingPage: Timeout activated");
}

void StationBrowsingPage::handle_enter_key() {
    _state->trigger_transition(_page, STREAM_PLAYING);
    _model.confirmed_selection = _model.selected;
    spdlog::info("StationBrowsingPage: Stream {1} selected; transitioning", _model.confirmed_selection);
}

void StationBrowsingPage::handle_wheel_input(int delta) {
    _model.selected += delta;
    if (_model.selected < 0) _model.selected += _model.stations.size();
    _model.selected %= _model.stations.size();
    if(_model.times_out)
        _model.remaining_time = BROWSING_TIMEOUT;
}

void StationBrowsingPage::render(Renderer &renderer) {
    if(_model.times_out && _model.remaining_time > 0){
        --_model.remaining_time;
    }
    if(_model.times_out && _model.remaining_time == 0) {
        _state->trigger_transition(_page, STREAM_PLAYING);
    }

    this->render_time(renderer);
    //each page contains up to four stations
    //render each of the stations in a loop and then place the page indicator below

    for (int i = _model.offset; i < _model.offset + _model.limit && i < _model.stations.size(); i++) {
        auto station = _model.stations[i];

        //0. calculate render offset for the four corners
        auto offsetTile = i % _model.limit;
        int offsetX = offsetTile % 2 == 0 ? 80 : 240, offsetY = offsetTile < 2 ? 62 : 152;

        //1. render a possible selection background
        if (_model.selected == i) {
            renderer.render_highlight(offsetX-80, offsetY-32, 160, 90);
        }

        //2. render the artwork
        SDL_Rect dstrect = {offsetX - 24, offsetY - 24, 48, 48};
        void* image_ptr = _res.get_cached(station.image_url);
        if(image_ptr == nullptr)
            image_ptr = _res.get_static(RADIO_IMAGE);
        auto image = reinterpret_cast<SDL_Surface*>(image_ptr);
        renderer.render_image(image, dstrect);

        //3. render the station name
        renderer.render_text_small(offsetX, offsetY+35, station.name);
    }

    // render the page indicators
    auto has_paging = false;
    if (_model.offset > 0) {
        //render left indicator
        SDL_Rect dstrect{4, 210, 24, 24};
        auto image = reinterpret_cast<SDL_Surface*>(_res.get_static(ARROW_RIGHT));
        renderer.render_image(image, dstrect);
        has_paging = true;
    }
    if (_model.offset + _model.limit < _model.stations.size()) {
        //render right indicator
        SDL_Rect dstrect{292, 210, 24, 24};
        auto image = reinterpret_cast<SDL_Surface*>(_res.get_static(ARROW_LEFT));
        renderer.render_image(image, dstrect);
        has_paging = true;
    }

    //render the page number
    if (has_paging) {
        int page_num = _model.offset / _model.limit;
        renderer.render_text_small(160, 225, std::to_string(page_num));
    }

}