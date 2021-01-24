#ifndef KITCHENSOUND_OPTIONS_PAGE_H
#define KITCHENSOUND_OPTIONS_PAGE_H

#include "kitchensound/render_page.h"

class OptionsPage : public BasePage {
public:
    explicit OptionsPage(StateController* ctrl);

    void enter_page(PAGES origin) override;
    void leave_page(PAGES destination) override;
    void handle_enter_key() override;
    void handle_wheel_input(int delta) override;

    void render(Renderer& renderer) override;

private:
    static std::string get_ip_addr() ;

    std::string _local_ip;
};

#endif //KITCHENSOUND_OPTIONS_PAGE_H
