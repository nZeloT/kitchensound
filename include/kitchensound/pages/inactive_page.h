#ifndef KITCHENSOUND_INACTIVE_PAGE_H
#define KITCHENSOUND_INACTIVE_PAGE_H

#include "kitchensound/pages/base_page.h"

class StateController;
class Renderer;

class InactivePage : public BasePage {
public:
    InactivePage(std::shared_ptr<StateController>& ctrl, int amp_power_gpio);
    ~InactivePage() override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override;
    void handle_wheel_input(int delta) override {};
    void handle_enter_key() override {};
    void handle_mode_key() override {};
    void handle_power_key() override;
    void render(std::unique_ptr<Renderer>& renderer) override;
private:
    struct InactivePageModel {
        std::time_t amp_cooldown_start;
        PAGES last_seen;
        int amp_power_gpio;
    } _model;
};

#endif //KITCHENSOUND_INACTIVE_PAGE_H
