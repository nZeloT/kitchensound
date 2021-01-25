#ifndef KITCHENSOUND_OPTIONS_PAGE_H
#define KITCHENSOUND_OPTIONS_PAGE_H

#include <ctime>

#include "kitchensound/pages/base_page.h"

class OptionsPage : public BasePage {
public:
    explicit OptionsPage(std::shared_ptr<StateController>& ctrl);
    ~OptionsPage() override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override;
    void handle_enter_key() override;
    void handle_wheel_input(int delta) override;

    void render(std::unique_ptr<Renderer>& renderer) override;

private:
    static std::string get_ip_addr();

    struct OptionsPageModel {
        std::string local_ip;
        std::time_t startup_time;
    } _model;
};

#endif //KITCHENSOUND_OPTIONS_PAGE_H
