#ifndef KITCHENSOUND_OPTIONS_PAGE_H
#define KITCHENSOUND_OPTIONS_PAGE_H

#include <vector>

#include "kitchensound/pages/base_page.h"

struct ApplicationBackbone;

class OsUtil;
class Timer;

class OptionsPage : public BasePage {
public:

    OptionsPage(ApplicationBackbone&, std::shared_ptr<OsUtil>&);
    ~OptionsPage() override;
    void enter_page(PAGES, void*) override;
    void* leave_page(PAGES destination) override;
    void handle_enter_key(InputEvent&) override;
    void handle_wheel_input(int delta) override;

    void render() override;

private:
    void update_model();

    std::shared_ptr<OsUtil> _os;

    std::unique_ptr<Timer> _value_update;

    struct {
        int selection_idx;
        int selection_offset;
        int selection_limit;
        std::vector<std::string> data;
    } _model;
};

#endif //KITCHENSOUND_OPTIONS_PAGE_H
