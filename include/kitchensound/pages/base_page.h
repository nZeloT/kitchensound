#ifndef KITCHENSOUND_BASE_PAGE_H
#define KITCHENSOUND_BASE_PAGE_H

#include <ctime>
#include <memory>

#include "kitchensound/pages/pages.h"

class StateController;

class Renderer;

class TimerManager;

class Timer;

struct InputEvent;

class BasePage {
public:
    virtual ~BasePage();

    virtual void enter_page(PAGES origin, void *payload) { this->update_time(); };

    virtual void handle_power_key(InputEvent&);

    virtual void handle_mode_key(InputEvent&);

    virtual void *leave_page(PAGES destination) = 0;

    virtual void handle_wheel_input(int delta) = 0;

    virtual void handle_enter_key(InputEvent&) = 0;

    virtual void render(Renderer &renderer) = 0;

    virtual int get_update_delay_time() { return _bp_model.update_delay_time; };

protected:
    BasePage(PAGES page, StateController &ctrl, TimerManager&);

    void render_time(Renderer &renderer) const;

    StateController &_state;

    PAGES _page;
private:
    friend class InactivePage;

    void update_time();

    Timer& _update_time;

    struct BasePageModel {
        int hour;
        int minute;

        int update_delay_time;
    } _bp_model;
};

#endif //KITCHENSOUND_BASE_PAGE_H
