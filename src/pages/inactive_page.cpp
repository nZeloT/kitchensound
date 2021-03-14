#include "kitchensound/pages/inactive_page.h"

#include <sstream>
#include <iomanip>

#include <spdlog/spdlog.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/input_event.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/time_based_standby.h"
#include "kitchensound/timer.h"
#include "kitchensound/timer_manager.h"
#include "kitchensound/gpio_util.h"

InactivePage::InactivePage(StateController& ctrl, TimerManager& tm, std::shared_ptr<TimeBasedStandby>& standby, std::shared_ptr<GpioUtil>& gpio)
    : BasePage(INACTIVE, ctrl, tm), _model{MENU_SELECTION, false, false, false, false},
    _standby{standby}, _gpio{gpio},
    _amp_cooldown_timer{tm.request_timer(AMPLIFIER_DELAY, false, [this](){
        this->_model.amp_cooldown_active = false;
        this->update_state();
    })},
    _user_active_timer{tm.request_timer(STANDBY_TIMEOUT, false, [this](){
        this->_model.user_cooldown_active = false;
        this->update_state();
    })}
{
    _standby->set_change_callback([this](auto new_state) {
        this->_model.standby_active = new_state;
        this->update_state();
    });
    _standby->arm();
}

InactivePage::~InactivePage() = default;

void InactivePage::enter_page(PAGES origin, void* payload) {
    this->update_time();
    _amp_cooldown_timer.reset();
    _model.amp_cooldown_active = true;
    _model.last_seen = origin;
    _gpio->turn_off_amplifier();
    _standby->arm();
    SPDLOG_INFO("Entered page.");
}

void* InactivePage::leave_page(PAGES destination) {
    _standby->disarm();
    _gpio->turn_on_display();
    _gpio->turn_on_amplifier();
    SPDLOG_INFO("Left page.");
    return nullptr;
}

void InactivePage::update_state() {
    if (!_model.user_cooldown_active
        && _model.standby_active
        && _model.display_on) {
        _gpio->turn_off_display();
        _model.display_on = false;
        _bp_model.update_delay_time = 500;
    }

    if (!_model.standby_active
        && !_model.display_on) {
        _gpio->turn_on_display();
        _model.display_on = true;
        _bp_model.update_delay_time = 20;
    }
}

void InactivePage::handle_power_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT) {
        _user_active_timer.reset();
        _model.user_cooldown_active = true;
        if (!_model.amp_cooldown_active)
            _state.trigger_transition(_page, _model.last_seen);
    }
}

void InactivePage::handle_wheel_input(int delta) {
    _user_active_timer.reset();
    _model.user_cooldown_active = true;
}

void InactivePage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT){
        _user_active_timer.reset();
        _model.user_cooldown_active = true;
    }
}

void InactivePage::handle_mode_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT){
        _user_active_timer.reset();
        _model.user_cooldown_active = true;
    }
}

void InactivePage::render(Renderer& renderer) {
    if(_model.standby_active)
        return;

    //if(!_model.display_on && !_standby->is_standby_active()) //TODO why was that needed?
    //    _gpio->turn_on_display();

    if (_model.amp_cooldown_active) {
        //display cooldown
        int width = std::ceil(320 * (1 - _amp_cooldown_timer->progress_percentage()));
        renderer.render_rect(0, 230, width, 10, Renderer::FOREGROUND);
    }

    //just display a large digital clock
    std::ostringstream time;
    time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minute < 10 ? "0" : "")
         << std::to_string(_bp_model.minute);

    renderer.render_text(160, 120, time.str(), Renderer::HUGHE);
}