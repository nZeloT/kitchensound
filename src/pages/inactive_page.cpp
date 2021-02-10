#include "kitchensound/pages/inactive_page.h"

#include <sstream>
#include <iomanip>
#include <utility>

#include <wiringPi.h>
#include <spdlog/spdlog.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"

InactivePage::InactivePage(std::shared_ptr<StateController>& ctrl, int amp_power_gpio) : BasePage(INACTIVE, ctrl), _model{} {
    _model.last_seen = MENU_SELECTION;
    _model.amp_power_gpio = amp_power_gpio;
}

InactivePage::~InactivePage() = default;

void InactivePage::enter_page(PAGES origin, void* payload) {
    this->update_time();
    _model.amp_cooldown_start = _bp_model.current_time;
    _model.last_seen = origin;
    //turn off amp
    digitalWrite(_model.amp_power_gpio, 1);
    spdlog::info("InactivePage::enter_page(): Entered.");
}

void* InactivePage::leave_page(PAGES destination) {
    //turn amp on
    digitalWrite(_model.amp_power_gpio, 0);
    spdlog::info("InactivePage::leave_page(): Left");
    return nullptr;
}

void InactivePage::handle_power_key() {
    if (_bp_model.current_time - _model.amp_cooldown_start >= AMPLIFIER_TIMEOUT)
        _state->trigger_transition(_page, _model.last_seen);
}

void InactivePage::render(std::unique_ptr<Renderer>& renderer) {
    auto remaining = _bp_model.current_time - _model.amp_cooldown_start;
    if (remaining < AMPLIFIER_TIMEOUT) {

        //display cooldown
        int width = std::ceil(320 * (AMPLIFIER_TIMEOUT - remaining + 0.0) / (AMPLIFIER_TIMEOUT + 0.0));
        renderer->render_foreground(0, 230, width, 10);
    }

    //just display a large digital clock
    std::ostringstream time;
    time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minute < 10 ? "0" : "")
         << std::to_string(_bp_model.minute);

    renderer->render_text_hughe(160, 120, time.str());
}