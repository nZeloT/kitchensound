#include "kitchensound/render_page.h"

#include <wiringPi.h>
#include <spdlog/spdlog.h>

#include "kitchensound/state_controller.h"
#include "kitchensound/timeouts.h"

void BasePage::handle_power_key() {
    _state->trigger_transition(_page, INACTIVE);
}

void BasePage::handle_mode_key() {
    _state->trigger_transition(_page, MENU_SELECTION);
}

void VolumePage::handle_wheel_input(int delta) {
    if (delta != 0) {
        _vol_model.active_change_timeout = VOLUME_TIMEOUT;
        auto new_val = _volume.get_volume() + delta;
        if (new_val < 0 || new_val > 100) return;
        _volume.apply_delta(delta);
    }
}

void VolumePage::render_volume(Renderer &renderer) {
    if (_vol_model.active_change_timeout >= 0)
        --_vol_model.active_change_timeout;
    if (_vol_model.active_change_timeout < 0)
        return;

    auto vol = _volume.get_volume();

    std::ostringstream volume;
    volume << std::to_string(vol);

    //render background
    renderer.render_highlight(290, 0, 30, 240);

    //render volume bar
    renderer.render_background(300, 10, 12, 200);

    //render the actual volume
    renderer.render_foreground(300, 10 + 200 - 2 * vol, 12, 2 * vol);

    //render the volume value
    renderer.render_text_small(305, 225, volume.str());
}

void VolumePage::enter_page(PAGES origin) {
    //_volume.update_from_system();
    //_vol_model.new_value = _volume.get_volume();
    spdlog::info("VolumePage::enter_page(): Entered.");
}

void LoadingPage::render(Renderer &renderer) {
    this->render_time(renderer);
    renderer.render_text_large(160, 120, _model.msg);
}

void InactivePage::enter_page(PAGES origin) {
    _model.amp_cooldown_start = _bp_model.current_time;
    _model.last_seen = origin;
    //turn off amp
    digitalWrite(4, 1);
    spdlog::info("InactivePage::enter_page(): Entered.");
}

void InactivePage::leave_page(PAGES destination) {
    //turn amp on
    digitalWrite(4, 0);
    spdlog::info("InactivePage::leave_page(): Left");
}

void InactivePage::handle_power_key() {
    if (_bp_model.current_time - _model.amp_cooldown_start >= AMPLIFIER_TIMEOUT)
        _state->trigger_transition(_page, _model.last_seen);
}

void InactivePage::render(Renderer &renderer) {
    auto remaining = _bp_model.current_time - _model.amp_cooldown_start;
    if (remaining < AMPLIFIER_TIMEOUT) {

        //display cooldown
        int width = std::ceil(320 * (AMPLIFIER_TIMEOUT - remaining + 0.0) / (AMPLIFIER_TIMEOUT + 0.0));
        renderer.render_foreground(0, 230, width, 10);
    }

    //just display a large digital clock
    std::ostringstream time;
    time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minute < 10 ? "0" : "")
         << std::to_string(_bp_model.minute);

    renderer.render_text_hughe(160, 120, time.str());
}



