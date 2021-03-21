#ifndef KITCHENSOUND_LOADING_PAGE_H
#define KITCHENSOUND_LOADING_PAGE_H

#include <string>

#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/base_page.h"

struct ApplicationBackbone;

class LoadingPage : public BasePage {
public:
    explicit LoadingPage(ApplicationBackbone&);
    ~LoadingPage() override;
    void enter_page(PAGES origin, void* payload) override;
    void handle_wheel_input(int delta) override {};
    void handle_enter_key(InputEvent&) override {};
    void render() override;

    void set_text(std::string const &text);

private:
    struct LoadingPageModel {
        std::string msg;
    } _model;
};

#endif //KITCHENSOUND_LOADING_PAGE_H
