#include "kitchensound/pages/base_page.h"

#include <sstream>
#include <iomanip>

#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"

BasePage::~BasePage() = default;

void BasePage::update_time() {
    _bp_model.current_time = std::time(nullptr);
    auto tmp = std::localtime(&_bp_model.current_time);
    _bp_model.hour = tmp->tm_hour;
    _bp_model.minute = tmp->tm_min;
}

void BasePage::render_time(std::unique_ptr<Renderer>& renderer) const {
    std::ostringstream time;
    time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minute < 10 ? "0" : "")
         << std::to_string(_bp_model.minute);
    renderer->render_text(160, 15, time.str(), Renderer::SMALL);
}

void BasePage::handle_power_key() {
    _state->trigger_transition(_page, INACTIVE);
}

void BasePage::handle_mode_key() {
    _state->trigger_transition(_page, MENU_SELECTION);
}