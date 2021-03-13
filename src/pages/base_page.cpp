#include "kitchensound/pages/base_page.h"

#include <sstream>
#include <iomanip>

#include "kitchensound/timeouts.h"
#include "kitchensound/input_event.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/timer.h"

BasePage::BasePage(PAGES page, StateController &ctrl)
    : _bp_model{0, 0, 20}, _state{ctrl},_page{page},
      _update_time_timer{std::make_unique<Timer>(CLOCK_UPDATE_DELAY, true, [this](){
          this->update_time();
      })}
    {}

BasePage::~BasePage() = default;

void BasePage::update(long ms_delta_time) {
    _update_time_timer->update(ms_delta_time);
}

void BasePage::update_time() {
    auto now = std::time(nullptr);
    auto tmp = std::localtime(&now);
    _bp_model.hour = tmp->tm_hour;
    _bp_model.minute = tmp->tm_min;
}

void BasePage::render_time(Renderer& renderer) const {
    std::ostringstream time;
    time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minute < 10 ? "0" : "")
         << std::to_string(_bp_model.minute);
    renderer.render_text(160, 15, time.str(), Renderer::SMALL);
}

void BasePage::handle_power_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT)
        _state.trigger_transition(_page, INACTIVE);
}

void BasePage::handle_mode_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT)
        _state.trigger_transition(_page, MENU_SELECTION);
}