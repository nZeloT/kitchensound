#ifndef KITCHENSOUND_LOADING_PAGE_H
#define KITCHENSOUND_LOADING_PAGE_H

#include <string>

#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/base_page.h"

class StateController;
class Renderer;
class TimerManager;

class LoadingPage : public BasePage {
public:
    LoadingPage(StateController& ctrl, TimerManager&);
    ~LoadingPage() override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override { return nullptr; };
    void handle_wheel_input(int delta) override {};
    void handle_enter_key(InputEvent&) override {};
    void render(Renderer& renderer) override;

    void set_text(std::string const &text);

private:
    struct LoadingPageModel {
        std::string msg;
    } _model;
};

#endif //KITCHENSOUND_LOADING_PAGE_H
