#include "kitchensound/render_page.h"

#include <wiringPi.h>
#include <spdlog/spdlog.h>

#include "kitchensound/state_controller.h"
#include "kitchensound/timeouts.h"

void BasePage::handle_power_key() {
    _state->trigger_transition(_page, INACTIVE);
}

void BasePage::handle_network_key() {
    if (_page == BT_PLAYING)
        _state->trigger_transition(_page, STREAM_BROWSING);
    else
        _state->trigger_transition(_page, BT_PLAYING);
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
    spdlog::info("Volume: Entered VolumePage.");
}

void LoadingPage::render(Renderer &renderer) {
    this->render_time(renderer);
    renderer.render_text_large(160, 120, _model.msg);
    spdlog::info("LoadingPage: rendered");
}

void InactivePage::enter_page(PAGES origin) {
    _model.remaining_amp_cooldown_time = COOLDOWN_TIMEOUT;
    //turn off amp
    digitalWrite(4, 1);
    spdlog::info("InactivePage: Entered InactivePage.");
}

void InactivePage::leave_page(PAGES destination) {
    //turn amp on
    digitalWrite(4, 0);
    spdlog::info("InactivePage: Left InactivePage");
}

void InactivePage::handle_power_key() {
    if (_model.remaining_amp_cooldown_time < 0)
        _state->trigger_transition(_page, STREAM_BROWSING);
}

void InactivePage::render(Renderer &renderer) {
    if (_model.remaining_amp_cooldown_time >= 0) {
        --_model.remaining_amp_cooldown_time;

        //display cooldown
        int width = std::ceil(320 * (_model.remaining_amp_cooldown_time + 0.0) / (COOLDOWN_TIMEOUT + 0.0));
        renderer.render_foreground(0, 230, width, 10);
    }

    //just display a large digital clock
    std::ostringstream time;
    time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minutes < 10 ? "0" : "")
         << std::to_string(_bp_model.minutes);

    renderer.render_text_hughe(160, 120, time.str());
}



