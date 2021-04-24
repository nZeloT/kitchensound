#include "kitchensound/state_controller.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/input_event.h"
#include "kitchensound/analytics_logger.h"
#include "kitchensound/pages/inactive_page.h"

#define MYCASE(x) case x: return #x;

static std::string to_string(PAGES page) {
    switch(page){
        MYCASE(PAGES::INACTIVE)
        MYCASE(PAGES::LOADING)
        MYCASE(PAGES::MENU_SELECTION)
        MYCASE(PAGES::STREAM_PLAYING)
        MYCASE(PAGES::STREAM_SELECTION)
        MYCASE(PAGES::BT_PLAYING)
        MYCASE(PAGES::SNAPCAST_PLAYING)
        MYCASE(PAGES::OPTIONS)
        default: return "Unknown Pages. Extend to_string() in StateController.";
    }
}

StateController::StateController(std::unique_ptr<AnalyticsLogger>& analytics)
    : _transitions{TRANSITION_STATE::NONE}, _transition_origin{PAGES::INACTIVE}, _transition_destination{PAGES::INACTIVE},
      _active_page{nullptr}, _previous_page{nullptr}, _next_page{nullptr}, _transition_payload{nullptr},
      _pages{}, _analytics{analytics}
{}

StateController::~StateController() = default;

void StateController::register_pages(std::unordered_map<PAGES, std::unique_ptr<BasePage>> pages) {
    _pages = std::move(pages);
}

void StateController::setup_inactive_page() {
    _active_page = _pages[PAGES::INACTIVE].get();
    auto inactive = reinterpret_cast<InactivePage*>(_active_page);
    inactive->setup_inital_state();
}

void StateController::trigger_transition(PAGES origin, PAGES destination) {
    if(origin == destination)
        return;

    if(_transitions != TRANSITION_STATE::NONE)
        throw std::runtime_error{"Can't transition while in transition!"};

    _transition_origin = origin;
    _transition_destination = destination;
    _transitions = TRANSITION_STATE::ENTER_LOADING;
    _analytics->log_page_change(_transition_origin, _transition_destination);
    SPDLOG_INFO("Triggered transition {0} -> {1}", to_string(origin), to_string(destination));

}

bool StateController::update() {
    if(_transitions != TRANSITION_STATE::NONE)
        return process_transition();
    return false;
}

void StateController::render() {
    _active_page->render();
}

bool StateController::process_transition() {
    SPDLOG_INFO("Switching state.");
    switch (_transitions) {
        case TRANSITION_STATE::LEAVING_LOADING:
            _active_page->leave_page(_transition_destination); // leave loading and ignore the payload
            _active_page = _next_page;
            _next_page = nullptr;
            _previous_page = nullptr;
            _transition_origin = _transition_destination = PAGES::INACTIVE;
            _transitions = TRANSITION_STATE::NONE;
            _transition_payload = nullptr;
            SPDLOG_INFO("Processed LEAVING_LOADING");
            break;

        case TRANSITION_STATE::ENTERING:
            transition_select_next_page();
            _next_page->enter_page(_transition_origin, _transition_payload);
            _transitions = TRANSITION_STATE::LEAVING_LOADING;
            SPDLOG_INFO("Processed ENTERING");
            break;

        case TRANSITION_STATE::LEAVING:
            _transition_payload = _previous_page->leave_page(_transition_destination);
            _transitions = TRANSITION_STATE::ENTERING;
            SPDLOG_INFO("Processed LEAVING");
            break;

        case TRANSITION_STATE::ENTER_LOADING:
            _previous_page = _active_page;
            _active_page   = _pages[PAGES::LOADING].get();
            _active_page->enter_page(_transition_origin, nullptr); // no payload for loading page
            _transitions = TRANSITION_STATE::LEAVING;
            SPDLOG_INFO("Processed ENTER_LOADING");
            break;

        case TRANSITION_STATE::NONE: break;
    }

    if(_transitions != TRANSITION_STATE::NONE)
        return true;
    return false;
}

void StateController::transition_select_next_page() {
    _next_page = _pages[_transition_destination].get();
    if(_next_page == nullptr)
        throw std::runtime_error{"Tried to transition to Loading or unknown!"};
}

void StateController::react_wheel_input(InputEvent& inev) {
    SPDLOG_DEBUG("React Wheel Input");
    _active_page->handle_wheel_input(inev.value);
}

void StateController::react_confirm(InputEvent& inev) {
    SPDLOG_DEBUG("React Confirm");
    _active_page->handle_enter_key(inev);
}

void StateController::react_power_change(InputEvent& inev) {
    SPDLOG_DEBUG("Rect Power Change");
    _active_page->handle_power_key(inev);
}

void StateController::react_menu_change(InputEvent& inev) {
    SPDLOG_DEBUG("React Menu Change");
    _active_page->handle_mode_key(inev);
}

MAKE_ENUM_STRINGIFY(ENUM_TRANSITION_STATE,StateController::TRANSITION_STATE)