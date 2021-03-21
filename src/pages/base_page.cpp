#include "kitchensound/pages/base_page.h"

#include <sstream>
#include <iomanip>
#include <ctime>

#include <spdlog/fmt/bundled/chrono.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/input_event.h"
#include "kitchensound/renderer.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/timer.h"

#include "kitchensound/application_backbone.h"

BasePage::BasePage(PAGES page, ApplicationBackbone &bb)
        : _bp_model{0, 0}, _bb{bb}, _page{page},
          _update_time{std::make_unique<Timer>(bb.fdreg, "BasePage Clock Update", CLOCK_UPDATE_DELAY, true, [this]() {
              this->update_time();
          })} {}

BasePage::~BasePage() = default;

void BasePage::enter_page(PAGES origin, void *playload) {
    _update_time->reset_timer();
    update_time();
}

void *BasePage::leave_page(PAGES destination) {
    _update_time->stop();
    return nullptr;
}

void BasePage::handle_power_key(InputEvent &inev) {
    if (inev.value == INEV_KEY_SHORT)
        _bb.ctrl->trigger_transition(_page, INACTIVE);
}

void BasePage::handle_mode_key(InputEvent &inev) {
    if (inev.value == INEV_KEY_SHORT)
        _bb.ctrl->trigger_transition(_page, MENU_SELECTION);
}

void BasePage::update_time() {
    const auto now = std::time(nullptr);
    auto tmp = std::localtime(&now);
    _bp_model.hour = tmp->tm_hour;
    _bp_model.minute = tmp->tm_min;
}

void BasePage::render_time() const {
    std::ostringstream time;
    time << std::setw(2) << std::to_string(_bp_model.hour) << " : " << (_bp_model.minute < 10 ? "0" : "")
         << std::to_string(_bp_model.minute);
    _bb.rend->render_text(160, 15, time.str(), Renderer::SMALL);
}
