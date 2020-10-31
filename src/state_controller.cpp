#include "kitchensound/state_controller.h"

#include <spdlog/spdlog.h>

StateController::StateController(Configuration &conf, ResourceManager& res, Renderer& renderer)
    : _transitions{NONE}, _transition_origin{INACTIVE}, _transition_destination{INACTIVE}, _renderer{renderer},
      _inactive{this}, _loading{this}, _volume{},
      _stream_browsing{this, res, std::move(conf.get_radio_stations())},
      _stream_playing{this, res, _volume},
      _bt_playing{this, res, _volume},
      _active_page{&_inactive}, _previous_page{nullptr}, _next_page{nullptr}
{
    _volume.set_volume(conf.get_volume());
}

StateController::~StateController() = default;

void StateController::trigger_transition(PAGES origin, PAGES destination) {
    if(origin == destination)
        return;

    if(_transitions != NONE)
        throw std::runtime_error{"Can't transition while in transition!"};

    _transition_origin = origin;
    _transition_destination = destination;
    _transitions = ENTER_LOADING;
    spdlog::info("StateController::trigger_transition(): Triggered Transition from {0} to {1}", origin, destination);

}

void StateController::update(bool time) {
    if (time)
        update_time();

    if(_transitions != NONE)
        process_transition();
}

void StateController::render() {
    _active_page->render(_renderer);
}

void StateController::process_transition() {
    spdlog::info("StateController::process_transition(): switching state");
    switch (_transitions) {
        case LEAVING_LOADING:
            _active_page->leave_page(_transition_destination);
            _active_page = _next_page;
            _next_page = nullptr;
            _previous_page = nullptr;
            _transition_origin = _transition_destination = INACTIVE;
            _transitions = NONE;
            spdlog::info("StateController::process_transition(): processed LEAVING_LOADING");
            break;

        case ENTERING:
            transition_select_next_page();
            _next_page->enter_page(_transition_origin);
            if(_transition_destination == STREAM_PLAYING) { //TODO maybe improve?
                auto stream = _stream_browsing.get_selected_stream();
                _stream_playing.set_station_playing(stream);
            }
            _transitions = LEAVING_LOADING;
            spdlog::info("StateController::process_transition(): processed ENTERING");
            break;

        case LEAVING:
            _previous_page->leave_page(_transition_destination);
            _transitions = ENTERING;
            spdlog::info("StateController::process_transition(): processed LEAVING");
            break;

        case ENTER_LOADING:
            _previous_page = _active_page;
            _active_page   = &_loading;
            _active_page->enter_page(_transition_origin);
            _transitions = LEAVING;
            spdlog::info("StateController::process_transition(): processed ENTER_LOADING");
            break;

        case NONE: break;
    }
}

void StateController::transition_select_next_page() {
    switch (_transition_destination) {
        case INACTIVE: _next_page = &_inactive; break;
        case STREAM_BROWSING: _next_page = &_stream_browsing; break;
        case STREAM_PLAYING: _next_page = &_stream_playing; break;
        case BT_PLAYING: _next_page = &_bt_playing; break;
        default:
            throw std::runtime_error{"Tried to transition to Loading or unknown!"};
    }
}

void StateController::update_time() {
    _active_page->update_time();
}

void StateController::react_wheel_input(int delta) {
    _active_page->handle_wheel_input(delta);
}

void StateController::react_confirm() {
    _active_page->handle_enter_key();
}

void StateController::react_power_change() {
    _active_page->handle_power_key();
}

void StateController::react_network_change() {
    _active_page->handle_network_key();
}
