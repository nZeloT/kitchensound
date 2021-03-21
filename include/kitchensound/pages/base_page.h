#ifndef KITCHENSOUND_BASE_PAGE_H
#define KITCHENSOUND_BASE_PAGE_H

#include <memory>

#include "kitchensound/pages/pages.h"

struct ApplicationBackbone;

class Timer;
struct InputEvent;

class BasePage {
public:
    virtual ~BasePage();

    virtual void enter_page(PAGES, void *);

    virtual void handle_power_key(InputEvent&);

    virtual void handle_mode_key(InputEvent&);

    virtual void *leave_page(PAGES);

    virtual void handle_wheel_input(int) = 0;

    virtual void handle_enter_key(InputEvent&) = 0;

    virtual void render() = 0;

protected:
    BasePage(PAGES page, ApplicationBackbone&);

    void render_time() const;

    ApplicationBackbone& _bb;
    PAGES _page;
private:
    friend class InactivePage;

    void update_time();

    std::unique_ptr<Timer> _update_time;

    struct BasePageModel {
        int hour;
        int minute;
    } _bp_model;
};

#endif //KITCHENSOUND_BASE_PAGE_H
