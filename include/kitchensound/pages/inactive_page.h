#ifndef KITCHENSOUND_INACTIVE_PAGE_H
#define KITCHENSOUND_INACTIVE_PAGE_H

#include "kitchensound/pages/base_page.h"

class StateController;
class Renderer;
class TimeBasedStandby;
class GpioUtil;

class InactivePage : public BasePage {
public:
    InactivePage(StateController& ctrl, std::shared_ptr<TimeBasedStandby>& standby, std::shared_ptr<GpioUtil>& gpio);
    ~InactivePage() override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override;
    void update() override;
    void handle_wheel_input(int delta) override;
    void handle_enter_key(InputEvent&) override;
    void handle_mode_key(InputEvent&) override;
    void handle_power_key(InputEvent&) override;
    void render(Renderer& renderer) override;
    void update_time() override;
private:
    std::shared_ptr<GpioUtil> _gpio;
    std::shared_ptr<TimeBasedStandby> _standby;
    struct InactivePageModel {
        std::time_t amp_cooldown_start;
        PAGES last_seen;
        bool display_on;
    } _model;
};

#endif //KITCHENSOUND_INACTIVE_PAGE_H
