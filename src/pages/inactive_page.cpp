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
#include "kitchensound/gpio_util.h"

InactivePage::InactivePage(StateController& ctrl, std::shared_ptr<TimeBasedStandby>& standby, std::shared_ptr<GpioUtil>& gpio)
    : BasePage(INACTIVE, ctrl), _model{MENU_SELECTION, false, false},
    _standby{standby}, _gpio{gpio},
    _amp_cooldown_timer{std::make_unique<Timer>(AMPLIFIER_DELAY, false, [this](){
        this->_model.amp_cooldown_active = false;
    })}
    {
        standby->arm();
    }

InactivePage::~InactivePage() = default;

void InactivePage::enter_page(PAGES origin, void* payload) {
    this->update_time();
    _amp_cooldown_timer->reset();
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

void InactivePage::update(long ms_delta_time) {
    _amp_cooldown_timer->update(ms_delta_time);
    if(_standby->is_standby_active()){
        if(_model.display_on){
            _gpio->turn_off_display();
            _model.display_on = false;
            _bp_model.update_delay_time = 500;
        }
    }else{
        if(!_model.display_on){
            _gpio->turn_on_display();
            _model.display_on = true;
            _bp_model.update_delay_time = 20;
        }
    }
    BasePage::update(ms_delta_time);
}

void InactivePage::handle_power_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT) {
        _standby->reset_standby_cooldown();
        if (!_model.amp_cooldown_active)
            _state.trigger_transition(_page, _model.last_seen);
    }
}

void InactivePage::handle_wheel_input(int delta) {
    _standby->reset_standby_cooldown();
}

void InactivePage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT)
        _standby->reset_standby_cooldown();
}

void InactivePage::handle_mode_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT)
        _standby->reset_standby_cooldown();
}

void InactivePage::update_time() {
    _standby->update_time();
    BasePage::update_time();
}

void InactivePage::render(Renderer& renderer) {
    if(_standby->is_standby_active())
        return;
    if(!_model.display_on && !_standby->is_standby_active())
        _gpio->turn_on_display();

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