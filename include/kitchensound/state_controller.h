#ifndef KITCHENSOUND_STATE_CONTROLLER_H
#define KITCHENSOUND_STATE_CONTROLLER_H

#include <memory>
#include <unordered_map>
#include <iostream>

#include "kitchensound/enum_helper.h"
#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/base_page.h"

#define ENUM_TRANSITION_STATE(DO,ACCESSOR)  \
    DO(NONE,ACCESSOR)                       \
    DO(LEAVING_LOADING,ACCESSOR)            \
    DO(ENTERING,ACCESSOR)                   \
    DO(LEAVING,ACCESSOR)                    \
    DO(ENTER_LOADING,ACCESSOR)              \


class Renderer;
struct InputEvent;
class AnalyticsLogger;

class StateController {
public:
    explicit StateController(std::unique_ptr<AnalyticsLogger>&);

    ~StateController();

    void register_pages(std::unordered_map<PAGES, std::unique_ptr<BasePage>> pages);

    void setup_inactive_page();

    bool update();

    void render();

    void trigger_transition(PAGES origin, PAGES destination);

    void react_wheel_input(InputEvent&);

    void react_menu_change(InputEvent&);

    void react_confirm(InputEvent&);

    void react_power_change(InputEvent&);

private:
    bool process_transition();

    void transition_select_next_page();

    enum class TRANSITION_STATE {
        ENUM_TRANSITION_STATE(MAKE_ENUM,)
    };
    friend std::ostream& operator<<(std::ostream&, StateController::TRANSITION_STATE val);

    BasePage* _active_page;
    BasePage* _previous_page;
    BasePage* _next_page;
    TRANSITION_STATE _transitions;
    PAGES _transition_origin;
    void* _transition_payload;
    PAGES _transition_destination;
    std::unordered_map<PAGES, std::unique_ptr<BasePage>> _pages;

    std::unique_ptr<AnalyticsLogger>& _analytics;
};

std::ostream& operator<<(std::ostream&, StateController::TRANSITION_STATE val);

#endif //KITCHENSOUND_STATE_CONTROLLER_H
