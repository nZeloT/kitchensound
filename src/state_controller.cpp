#include "kitchensound/state_controller.h"

#include <spdlog/spdlog.h>

#include "kitchensound/config.h"
#include "kitchensound/time_based_standby.h"

StateController::StateController(std::unique_ptr<Configuration>& conf)
    : _transitions{NONE}, _transition_origin{INACTIVE}, _transition_destination{INACTIVE},
      _active_page{nullptr}, _previous_page{nullptr}, _next_page{nullptr}, _transition_payload{nullptr},
      _standby{std::make_unique<TimeBasedStandby>(conf->get_display_standby())},
      _pages{}
{
    _standby->arm();
}

StateController::~StateController() = default;

void StateController::register_pages(std::unordered_map<PAGES, std::unique_ptr<BasePage>> pages) {
    _pages = std::move(pages);
}

void StateController::set_active_page(PAGES page) {
    _active_page = _pages[page].get();
}

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

void StateController::render(std::unique_ptr<Renderer>& renderer) {
    _active_page->render(renderer);
}

void StateController::process_transition() {
    spdlog::info("StateController::process_transition(): switching state");
    switch (_transitions) {
        case LEAVING_LOADING:
            _active_page->leave_page(_transition_destination); // leave loading and ignore the payload
            _active_page = _next_page;
            _next_page = nullptr;
            _previous_page = nullptr;
            _transition_origin = _transition_destination = INACTIVE;
            _transitions = NONE;
            _transition_payload = nullptr;
            spdlog::info("StateController::process_transition(): processed LEAVING_LOADING");
            break;

        case ENTERING:
            transition_select_next_page();
            _next_page->enter_page(_transition_origin, _transition_payload);
            if(_transition_destination == INACTIVE)
                _standby->arm();
            _transitions = LEAVING_LOADING;
            spdlog::info("StateController::process_transition(): processed ENTERING");
            break;

        case LEAVING:
            _transition_payload = _previous_page->leave_page(_transition_destination);
            if(_transition_origin == INACTIVE)
                _standby->disarm();
            _transitions = ENTERING;
            spdlog::info("StateController::process_transition(): processed LEAVING");
            break;

        case ENTER_LOADING:
            _previous_page = _active_page;
            _active_page   = _pages[LOADING].get();
            _active_page->enter_page(_transition_origin, nullptr); // no payload for loading page
            _transitions = LEAVING;
            spdlog::info("StateController::process_transition(): processed ENTER_LOADING");
            break;

        case NONE: break;
    }
}

void StateController::transition_select_next_page() {
    _next_page = _pages[_transition_destination].get();
    if(_next_page == nullptr)
        throw std::runtime_error{"Tried to transition to Loading or unknown!"};
}

void StateController::update_time() {
    _active_page->update_time();
    _standby->update_time();
}

void StateController::react_wheel_input(int delta) {
    _active_page->handle_wheel_input(delta);
    _standby->reset_standby_cooldown();
}

void StateController::react_confirm() {
    _active_page->handle_enter_key();
    _standby->reset_standby_cooldown();
}

void StateController::react_power_change() {
    _active_page->handle_power_key();
    _standby->reset_standby_cooldown();
}

void StateController::react_menu_change() {
    _active_page->handle_mode_key();
    _standby->reset_standby_cooldown();
}

bool StateController::is_standby_active() {
    return _standby->is_standby_active();
}