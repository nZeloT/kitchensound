#ifndef KITCHENSOUND_INACTIVE_PAGE_H
#define KITCHENSOUND_INACTIVE_PAGE_H

#include <memory>

#include "kitchensound/pages/base_page.h"

class StateController;
class Renderer;
class TimeBasedStandby;
class GpioUtil;
class Timer;

class InactivePage : public BasePage {
public:
    InactivePage(StateController& ctrl, std::shared_ptr<TimeBasedStandby>& standby, std::shared_ptr<GpioUtil>& gpio);
    ~InactivePage() override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override;
    void update(long ms_delta_time) override;
    void handle_wheel_input(int delta) override;
    void handle_enter_key(InputEvent&) override;
    void handle_mode_key(InputEvent&) override;
    void handle_power_key(InputEvent&) override;
    void render(Renderer& renderer) override;
    void update_time() override;
private:

    std::shared_ptr<GpioUtil> _gpio;
    std::shared_ptr<TimeBasedStandby> _standby;

    std::unique_ptr<Timer> _amp_cooldown_timer;

    struct InactivePageModel {
        PAGES last_seen;
        bool display_on;
        bool amp_cooldown_active;
    } _model;
};

#endif //KITCHENSOUND_INACTIVE_PAGE_H
