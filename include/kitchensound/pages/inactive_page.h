#ifndef KITCHENSOUND_INACTIVE_PAGE_H
#define KITCHENSOUND_INACTIVE_PAGE_H

#include <memory>

#include "kitchensound/pages/base_page.h"

class StateController;
class Renderer;
class TimeBasedStandby;
class GpioUtil;
class Timer;
class TimerManager;

class InactivePage : public BasePage {
public:
    InactivePage(StateController& ctrl, TimerManager& tm, std::shared_ptr<TimeBasedStandby>& standby, std::shared_ptr<GpioUtil>& gpio);
    ~InactivePage() override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override;
    void handle_wheel_input(int delta) override;
    void handle_enter_key(InputEvent&) override;
    void handle_mode_key(InputEvent&) override;
    void handle_power_key(InputEvent&) override;
    void render(Renderer& renderer) override;
private:
    void update_state();

    std::shared_ptr<GpioUtil> _gpio;
    std::shared_ptr<TimeBasedStandby> _standby;

    Timer& _amp_cooldown_timer;
    Timer& _user_active_timer;

    struct InactivePageModel {
        PAGES last_seen;
        bool display_on;
        bool amp_cooldown_active;
        bool user_cooldown_active;
        bool standby_active;
    } _model;
};

#endif //KITCHENSOUND_INACTIVE_PAGE_H
