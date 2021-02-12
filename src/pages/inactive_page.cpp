#include "kitchensound/pages/inactive_page.h"

#include <sstream>
#include <iomanip>

#include <spdlog/spdlog.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"

InactivePage::InactivePage(StateController& ctrl, std::function<void(bool)> set_amp_state)
    : BasePage(INACTIVE, ctrl), _model{-1, MENU_SELECTION, std::move(set_amp_state)} {}

InactivePage::~InactivePage() = default;

void InactivePage::enter_page(PAGES origin, void* payload) {
    this->update_time();
    _model.amp_cooldown_start = _bp_model.current_time;
    _model.last_seen = origin;
    //turn off amp
    _model.set_amplifier_state(false);
    spdlog::info("InactivePage::enter_page(): Entered.");
}

void* InactivePage::leave_page(PAGES destination) {
    //turn amp on
    _model.set_amplifier_state(true);
    spdlog::info("InactivePage::leave_page(): Left");
    return nullptr;
}

void InactivePage::handle_power_key() {
    if (_bp_model.current_time - _model.amp_cooldown_start >= AMPLIFIER_TIMEOUT)
        _state.trigger_transition(_page, _model.last_seen);
}

void InactivePage::render(Renderer& renderer) {
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