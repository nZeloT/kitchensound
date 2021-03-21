#ifndef KITCHENSOUND_INACTIVE_PAGE_H
#define KITCHENSOUND_INACTIVE_PAGE_H

#include <memory>

#include "kitchensound/pages/base_page.h"

struct ApplicationBackbone;
class Renderer;
class TimeBasedStandby;
class GpioUtil;
class Timer;

class InactivePage : public BasePage {
public:
    InactivePage(ApplicationBackbone&, std::shared_ptr<TimeBasedStandby>&, std::shared_ptr<GpioUtil>&);
    ~InactivePage() override;
    void enter_page(PAGES, void*) override;
    void* leave_page(PAGES) override;
    void handle_wheel_input(int) override;
    void handle_enter_key(InputEvent&) override;
    void handle_mode_key(InputEvent&) override;
    void handle_power_key(InputEvent&) override;
    void render() override;
private:
    friend class StateController;
    void setup_inital_state();
    void update_state();

    std::shared_ptr<GpioUtil> _gpio;
    std::shared_ptr<TimeBasedStandby> _standby;

    std::unique_ptr<Timer> _amp_cooldown_timer;
    std::unique_ptr<Timer> _user_active_timer;

    struct InactivePageModel {
        PAGES last_seen;
        bool display_on;
        bool amp_cooldown_active;
        bool user_cooldown_active;
        bool standby_active;
    } _model;
};

#endif //KITCHENSOUND_INACTIVE_PAGE_H
