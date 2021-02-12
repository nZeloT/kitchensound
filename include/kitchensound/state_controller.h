#ifndef KITCHENSOUND_STATE_CONTROLLER_H
#define KITCHENSOUND_STATE_CONTROLLER_H

#include <memory>
#include <unordered_map>

#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/base_page.h"

class Volume;
class ResourceManager;
class Renderer;
class Volume;
class TimeBasedStandby;

class StateController {
public:
    explicit StateController(TimeBasedStandby& standby);

    ~StateController();

    void register_pages(std::unordered_map<PAGES, std::unique_ptr<BasePage>> pages);

    void set_active_page(PAGES page);

    void update(bool time);

    void render(Renderer& renderer);

    void trigger_transition(PAGES origin, PAGES destination);

    void react_wheel_input(int delta);

    void react_menu_change();

    void react_confirm();

    void react_power_change();

private:
    void update_time();

    void process_transition();

    void transition_select_next_page();

    enum TRANSITION_STATE {
        NONE,
        LEAVING_LOADING,
        ENTERING,
        LEAVING,
        ENTER_LOADING
    };

    BasePage* _active_page;
    BasePage* _previous_page;
    BasePage* _next_page;
    TRANSITION_STATE _transitions;
    PAGES _transition_origin;
    void* _transition_payload;
    PAGES _transition_destination;
    std::unordered_map<PAGES, std::unique_ptr<BasePage>> _pages;

    TimeBasedStandby& _standby; // TODO make page
};

#endif //KITCHENSOUND_STATE_CONTROLLER_H
