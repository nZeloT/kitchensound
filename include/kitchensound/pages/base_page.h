#ifndef KITCHENSOUND_BASE_PAGE_H
#define KITCHENSOUND_BASE_PAGE_H

#include <ctime>
#include <memory>

#include "kitchensound/pages/pages.h"

class StateController;

class Renderer;

class BasePage {
public:
    virtual ~BasePage();

    virtual void enter_page(PAGES origin, void *payload) { this->update_time(); };

    virtual void handle_power_key();

    virtual void handle_mode_key();

    virtual void *leave_page(PAGES destination) = 0;

    virtual void handle_wheel_input(int delta) = 0;

    virtual void handle_enter_key() = 0;

    virtual void render(Renderer &renderer) = 0;

    virtual void update();

    virtual int get_update_delay_time() { return _bp_model.update_delay_time; };

protected:
    BasePage(PAGES page, StateController &ctrl) : _bp_model{0, 0, 0, 20, 250, 0}, _state{ctrl}, _page{page} {};

    void render_time(Renderer &renderer) const;

    virtual void update_time();

    StateController &_state;
    PAGES _page;

private:
    friend class InactivePage;

    struct BasePageModel {
        std::time_t current_time;
        int hour;
        int minute;

        int update_delay_time;
        int update_time_frame_skip;
        int update_time_frame_cnt;
    } _bp_model;
};

#endif //KITCHENSOUND_BASE_PAGE_H
