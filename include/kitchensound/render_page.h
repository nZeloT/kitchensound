#ifndef KITCHENSOUND_RENDER_PAGE_H
#define KITCHENSOUND_RENDER_PAGE_H

#include <sstream>
#include <iomanip>
#include <memory>

#include "kitchensound/model.h"
#include "kitchensound/renderer.h"
#include "kitchensound/volume.h"

class StateController;

class BasePage {
public:
    virtual void enter_page(PAGES origin) { this->update_time(); };
    virtual void handle_power_key();
    virtual void handle_network_key();

    virtual void leave_page(PAGES destination) = 0;
    virtual void handle_wheel_input(int delta) = 0;
    virtual void handle_enter_key() = 0;
    virtual void render(Renderer& renderer) = 0;

    void update_time() {
        auto now = std::time(nullptr);
        auto local_time = std::localtime(&now);
        _bp_model.hour = local_time->tm_hour;
        _bp_model.minutes = local_time->tm_min;
    }

protected:
    BasePage(PAGES page, StateController* ctrl) : _bp_model{}, _state{ctrl}, _page{page} {};

    void render_time(Renderer &renderer) const {
        std::ostringstream time;
        time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minutes < 10 ? "0" : "")
             << std::to_string(_bp_model.minutes);
        renderer.render_text_small(160, 15, time.str());
    }

    StateController* _state;
    PAGES _page;

private:
    friend class InactivePage;
    BasePageModel _bp_model;
};

class VolumePage : public BasePage {
public:
    VolumePage(PAGES page, StateController* ctrl, Volume& vol) : BasePage(page, ctrl), _volume{vol}, _vol_model{} {};
    void enter_page(PAGES origin) override;
    void handle_wheel_input(int delta) override;

protected:
    void render_volume(Renderer &renderer);
private:
    Volume& _volume;
    VolumeModel _vol_model;
};

class InactivePage : public BasePage {
public:
    explicit InactivePage(StateController* ctrl) : BasePage(INACTIVE, ctrl), _model{} {};

    void enter_page(PAGES origin) override;
    void leave_page(PAGES destination) override;
    void handle_wheel_input(int delta) override {};
    void handle_enter_key() override {};
    void handle_network_key() override {};
    void handle_power_key() override;
    void render(Renderer &renderer) override;

private:
    InactivePageModel _model;
};

class LoadingPage : public BasePage {
public:
    explicit LoadingPage(StateController* ctrl) : BasePage(LOADING, ctrl), _model{} {};
    void enter_page(PAGES origin) override {
        this->update_time();
        _model.msg = "Loading...";
    }
    void leave_page(PAGES destination) override {};
    void handle_wheel_input(int delta) override {};
    void handle_enter_key() override {};
    void render(Renderer &renderer) override;

    void set_text(std::string const &text) {
        _model.msg = std::string{text};
    }

private:
    LoadingPageModel _model;
};

#endif //KITCHENSOUND_RENDER_PAGE_H
