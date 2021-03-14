#include "kitchensound/state_controller.h"

#include <spdlog/spdlog.h>

#include "kitchensound/sdl_util.h"
#include "kitchensound/input_event.h"

StateController::StateController()
    : _transitions{NONE}, _transition_origin{INACTIVE}, _transition_destination{INACTIVE},
      _active_page{nullptr}, _previous_page{nullptr}, _next_page{nullptr}, _transition_payload{nullptr},
      _pages{}
{}

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
    SPDLOG_INFO("Triggered transition {0} -> {1}", origin, destination);

}

void StateController::update() {
    if(_transitions != NONE)
        process_transition();
}

void StateController::render(Renderer& renderer) {
    _active_page->render(renderer);
}

void StateController::delay_next_frame() {
    delay(_active_page->get_update_delay_time());
}

void StateController::process_transition() {
    SPDLOG_INFO("Switching state.");
    switch (_transitions) {
        case LEAVING_LOADING:
            _active_page->leave_page(_transition_destination); // leave loading and ignore the payload
            _active_page = _next_page;
            _next_page = nullptr;
            _previous_page = nullptr;
            _transition_origin = _transition_destination = INACTIVE;
            _transitions = NONE;
            _transition_payload = nullptr;
            SPDLOG_INFO("Processed LEAVING_LOADING");
            break;

        case ENTERING:
            transition_select_next_page();
            _next_page->enter_page(_transition_origin, _transition_payload);
            _transitions = LEAVING_LOADING;
            SPDLOG_INFO("Processed ENTERING");
            break;

        case LEAVING:
            _transition_payload = _previous_page->leave_page(_transition_destination);
            _transitions = ENTERING;
            SPDLOG_INFO("Processed LEAVING");
            break;

        case ENTER_LOADING:
            _previous_page = _active_page;
            _active_page   = _pages[LOADING].get();
            _active_page->enter_page(_transition_origin, nullptr); // no payload for loading page
            _transitions = LEAVING;
            SPDLOG_INFO("Processed ENTER_LOADING");
            break;

        case NONE: break;
    }
}

void StateController::transition_select_next_page() {
    _next_page = _pages[_transition_destination].get();
    if(_next_page == nullptr)
        throw std::runtime_error{"Tried to transition to Loading or unknown!"};
}

void StateController::react_wheel_input(InputEvent& inev) {
    _active_page->handle_wheel_input(inev.value);
}

void StateController::react_confirm(InputEvent& inev) {
    _active_page->handle_enter_key(inev);
}

void StateController::react_power_change(InputEvent& inev) {
    _active_page->handle_power_key(inev);
}

void StateController::react_menu_change(InputEvent& inev) {
    _active_page->handle_mode_key(inev);
}