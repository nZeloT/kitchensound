#ifndef KITCHENSOUND_OPTIONS_PAGE_H
#define KITCHENSOUND_OPTIONS_PAGE_H

#include <vector>

#include "kitchensound/pages/base_page.h"

struct ApplicationBackbone;

class OsUtil;
class SongFaver;

class OptionsPage : public BasePage {
public:

    OptionsPage(ApplicationBackbone&, std::shared_ptr<OsUtil>&, std::shared_ptr<SongFaver>&);
    ~OptionsPage() override;
    void enter_page(PAGES, void*) override;
    void* leave_page(PAGES destination) override;
    void handle_enter_key(InputEvent&) override;
    void handle_wheel_input(int delta) override;

    void render() override;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif //KITCHENSOUND_OPTIONS_PAGE_H
