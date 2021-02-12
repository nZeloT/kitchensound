#ifndef KITCHENSOUND_INACTIVE_PAGE_H
#define KITCHENSOUND_INACTIVE_PAGE_H

#include <functional>

#include "kitchensound/pages/base_page.h"

class StateController;
class Renderer;

class InactivePage : public BasePage {
public:
    //TODO move standby here
    InactivePage(StateController& ctrl, std::function<void(bool)> set_amplifier_state);
    ~InactivePage() override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override;
    void handle_wheel_input(int delta) override {};
    void handle_enter_key() override {};
    void handle_mode_key() override {};
    void handle_power_key() override;
    void render(Renderer& renderer) override;
private:
    struct InactivePageModel {
        std::time_t amp_cooldown_start;
        PAGES last_seen;
        std::function<void(bool)> set_amplifier_state;
    } _model;
};

#endif //KITCHENSOUND_INACTIVE_PAGE_H
