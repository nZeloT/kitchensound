#ifndef KITCHENSOUND_INACTIVE_PAGE_H
#define KITCHENSOUND_INACTIVE_PAGE_H

#include "kitchensound/config.h"
#include "kitchensound/pages/base_page.h"

class StateController;
class Renderer;
class TimeBasedStandby;
class GpioUtil;

class InactivePage : public BasePage {
public:
    InactivePage(StateController& ctrl, Configuration::DisplayStandbyConfig standby, int display_gpio, int amplifier_gpio);
    ~InactivePage() override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override;
    void update() override;
    void handle_wheel_input(int delta) override;
    void handle_enter_key() override;
    void handle_mode_key() override;
    void handle_power_key() override;
    void render(Renderer& renderer) override;
    void update_time() override;
private:
    std::unique_ptr<TimeBasedStandby> _standby;
    std::unique_ptr<GpioUtil> _gpio;
    struct InactivePageModel {
        std::time_t amp_cooldown_start;
        PAGES last_seen;
        bool display_on;
    } _model;
};

#endif //KITCHENSOUND_INACTIVE_PAGE_H
