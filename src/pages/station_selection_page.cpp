#include "kitchensound/pages/station_selection_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/state_controller.h"

#define RADIO_IMAGE "img/radio.png"

StationSelectionPage::StationSelectionPage(StateController *ctrl, ResourceManager &res,
                                           std::vector<RadioStationStream> streams) :
        SelectionPage<RadioStationStream>(STREAM_SELECTION, ctrl, res, std::move(streams),
                                          [](ResourceManager &r, const RadioStationStream& s) {
                                              void* image_ptr = r.get_cached(s.image_url);
                                              if(image_ptr == nullptr)
                                                  image_ptr = r.get_static(RADIO_IMAGE);
                                              return image_ptr;
                                          },
                                          [](const RadioStationStream& s) {
                                              return s.name;
                                          }), _model{} {};

void StationSelectionPage::activate_timeout() {
    _model.times_out = true;
    _model.remaining_time = BROWSING_TIMEOUT;
    spdlog::info("StationSelectionPage::activate_timeout(): active");
}

void StationSelectionPage::handle_enter_key() {
    _state->trigger_transition(_page, STREAM_PLAYING);
    _model.confirmed_selection = _sp_model.selected;
    spdlog::info("StationSelectionPage::handle_enter_key(): Stream {0} selected; transitioning",
                 _model.confirmed_selection);
}

void StationSelectionPage::handle_wheel_input(int delta) {
    SelectionPage<RadioStationStream>::handle_wheel_input(delta);
    if (_model.times_out)
        _model.remaining_time = BROWSING_TIMEOUT;
}

void StationSelectionPage::render(Renderer &renderer) {
    if (_model.times_out && _model.remaining_time > 0) {
        --_model.remaining_time;
    }
    if (_model.times_out && _model.remaining_time == 0) {
        _state->trigger_transition(_page, STREAM_PLAYING);
    }

    SelectionPage<RadioStationStream>::render(renderer);
}