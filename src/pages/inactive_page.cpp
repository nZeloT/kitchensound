#include "kitchensound/pages/inactive_page.h"

#include <sstream>
#include <iomanip>

#include <spdlog/spdlog.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/time_based_standby.h"
#include "kitchensound/gpio_util.h"

InactivePage::InactivePage(StateController& ctrl, std::shared_ptr<TimeBasedStandby>& standby, std::shared_ptr<GpioUtil>& gpio)
    : BasePage(INACTIVE, ctrl), _model{-1, MENU_SELECTION, false},
    _standby{standby}, _gpio{gpio}
    {}

InactivePage::~InactivePage() = default;

void InactivePage::enter_page(PAGES origin, void* payload) {
    this->update_time();
    _model.amp_cooldown_start = _bp_model.current_time;
    _model.last_seen = origin;
    _gpio->turn_off_amplifier();
    _standby->arm();
    spdlog::info("InactivePage::enter_page(): Entered.");
}

void* InactivePage::leave_page(PAGES destination) {
    _standby->disarm();
    _gpio->turn_on_display();
    _gpio->turn_on_amplifier();
    spdlog::info("InactivePage::leave_page(): Left");
    return nullptr;
}

void InactivePage::update() {
    if(_standby->is_standby_active()){
        if(_model.display_on){
            _gpio->turn_off_display();
            _model.display_on = false;
            _bp_model.update_time_frame_skip = 30;
            _bp_model.update_delay_time = 500;
        }
    }else{
        if(!_model.display_on){
            _gpio->turn_on_display();
            _model.display_on = true;
            _bp_model.update_time_frame_skip = 250;
            _bp_model.update_delay_time = 20;
        }
    }
    BasePage::update();
}

void InactivePage::handle_power_key() {
    _standby->reset_standby_cooldown();
    if (_bp_model.current_time - _model.amp_cooldown_start >= AMPLIFIER_TIMEOUT)
        _state.trigger_transition(_page, _model.last_seen);
}

void InactivePage::handle_wheel_input(int delta) {
    _standby->reset_standby_cooldown();
}

void InactivePage::handle_enter_key() {
    _standby->reset_standby_cooldown();
}

void InactivePage::handle_mode_key() {
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

    auto remaining = _bp_model.current_time - _model.amp_cooldown_start;
    if (remaining < AMPLIFIER_TIMEOUT) {

        //display cooldown
        int width = std::ceil(320 * (AMPLIFIER_TIMEOUT - remaining + 0.0) / (AMPLIFIER_TIMEOUT + 0.0));
        renderer.render_rect(0, 230, width, 10, Renderer::FOREGROUND);
    }

    //just display a large digital clock
    std::ostringstream time;
    time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minute < 10 ? "0" : "")
         << std::to_string(_bp_model.minute);

    renderer.render_text(160, 120, time.str(), Renderer::HUGHE);
}