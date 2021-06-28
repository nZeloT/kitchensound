#include "kitchensound/pages/inactive_page.h"

#include <sstream>
#include <iomanip>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/input_event.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/time_based_standby.h"
#include "kitchensound/timer.h"
#include "kitchensound/gpio_util.h"
#include "kitchensound/application_backbone.h"

InactivePage::InactivePage(ApplicationBackbone& bb, std::shared_ptr<TimeBasedStandby>& standby, std::shared_ptr<GpioUtil>& gpio)
    : BasePage(PAGES::INACTIVE, bb), _model{PAGES::MENU_SELECTION, false, false, false, false},
    _standby{standby}, _gpio{gpio},
    _amp_cooldown_timer{std::make_unique<Timer>(bb.fdreg, "Inactive Page Amplifier Cooldown", AMPLIFIER_DELAY, false, [this](){
        this->_model.amp_cooldown_active = false;
        this->update_state();
    })},
    _user_active_timer{std::make_unique<Timer>(bb.fdreg, "Inactive Page Standby Block", STANDBY_TIMEOUT, false, [this]() {
        this->_model.user_cooldown_active = false;
        this->update_state();
    })}
{
    _standby->set_change_callback([this](auto new_state) {
        this->_model.standby_active = new_state;
        this->update_state();
    });
}

InactivePage::~InactivePage() {
    SPDLOG_DEBUG("Dropped Inactive Page");
}

void InactivePage::setup_inital_state() {
    _standby->arm();
    update_state();
    BasePage::enter_page(PAGES::MENU_SELECTION, nullptr);
}

void InactivePage::enter_page(PAGES origin, void* payload) {
    _user_active_timer->reset_timer();
    _amp_cooldown_timer->reset_timer();
    _model.amp_cooldown_active = true;
    _model.last_seen = origin;
    _gpio->turn_off_amplifier();
    _standby->arm();
    BasePage::enter_page(origin, payload);
    SPDLOG_INFO("Entered page.");
}

void* InactivePage::leave_page(PAGES destination) {
    BasePage::leave_page(destination);
    _user_active_timer->stop();
    _standby->disarm();
    _gpio->turn_on_display();
    _gpio->turn_on_amplifier();
    SPDLOG_INFO("Left page.");
    return nullptr;
}

void InactivePage::update_state() {
    SPDLOG_INFO("_model.user_cooldown_active == {}", _model.user_cooldown_active);
    SPDLOG_INFO("_model.standby_active == {}", _model.standby_active);

    if (!_model.user_cooldown_active
        && _model.standby_active
        && _model.display_on) {
        _gpio->turn_off_display();
        _model.display_on = false;
    }

    if ((_model.user_cooldown_active
    ||  !_model.standby_active)
        && !_model.display_on) {
        _gpio->turn_on_display();
        _model.display_on = true;
    }

    SPDLOG_INFO("_model.display_on == {}", _model.display_on);
}

void InactivePage::handle_power_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT) {
        _user_active_timer->reset_timer();
        _model.user_cooldown_active = true;
        this->update_state();
        if (!_model.amp_cooldown_active)
            _bb.ctrl->trigger_transition(_page, _model.last_seen);
    }else{
        SPDLOG_DEBUG("Received Long Press on Power Key");
    }
}

void InactivePage::handle_wheel_input(int delta) {
    _user_active_timer->reset_timer();
    _model.user_cooldown_active = true;
    this->update_state();
}

void InactivePage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT){
        _user_active_timer->reset_timer();
        _model.user_cooldown_active = true;
        this->update_state();
    }
}

void InactivePage::handle_mode_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT){
        _user_active_timer->reset_timer();
        _model.user_cooldown_active = true;
        this->update_state();
    }
}

void InactivePage::render() {
    if(!_model.display_on)
        return;

    auto& renderer = _bb.rend;

    if (_model.amp_cooldown_active) {
        //display cooldown
        renderer->render_text(160, 225, "Letting amp caps discharge.", Renderer::TEXT_SIZE::SMALL);
    }

    //just display a large digital clock
    std::ostringstream time;
    time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minute < 10 ? "0" : "")
         << std::to_string(_bp_model.minute);

    renderer->render_text(160, 120, time.str(), Renderer::TEXT_SIZE::HUGHE);
}