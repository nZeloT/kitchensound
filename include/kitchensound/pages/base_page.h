#ifndef KITCHENSOUND_BASE_PAGE_H
#define KITCHENSOUND_BASE_PAGE_H

#include <ctime>
#include <memory>
#include <utility>

#include "kitchensound/pages/pages.h"

class StateController;
class Renderer;

class BasePage {
public:
    virtual ~BasePage();
    virtual void enter_page(PAGES origin, void* payload) { this->update_time(); };
    virtual void handle_power_key();

    virtual void handle_mode_key();
    virtual void* leave_page(PAGES destination) = 0;
    virtual void handle_wheel_input(int delta) = 0;
    virtual void handle_enter_key() = 0;

    virtual void render(std::unique_ptr<Renderer>& renderer) = 0;

    virtual void update_time();

    PAGES get_kind() { return _page; };
protected:
    BasePage(PAGES page, std::shared_ptr<StateController>& ctrl) : _bp_model{}, _state{ctrl}, _page{page} {};
    void render_time(std::unique_ptr<Renderer>& renderer) const;

    std::shared_ptr<StateController> _state;
    PAGES _page;

private:
    friend class InactivePage;
    struct BasePageModel {
        std::time_t current_time = 0;
        int hour = 0;
        int minute = 0;
    } _bp_model;
};

#endif //KITCHENSOUND_BASE_PAGE_H
