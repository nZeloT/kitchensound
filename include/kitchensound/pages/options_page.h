#ifndef KITCHENSOUND_OPTIONS_PAGE_H
#define KITCHENSOUND_OPTIONS_PAGE_H

#include "kitchensound/pages/base_page.h"

class OsUtil;

class OptionsPage : public BasePage {
public:
    OptionsPage(StateController& ctrl, std::shared_ptr<OsUtil>&);
    ~OptionsPage() override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override;
    void handle_enter_key(InputEvent&) override;
    void handle_wheel_input(int delta) override;

    void render(Renderer& renderer) override;

private:
    std::shared_ptr<OsUtil> _os;
};

#endif //KITCHENSOUND_OPTIONS_PAGE_H
