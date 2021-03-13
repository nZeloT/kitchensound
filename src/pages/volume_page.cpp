#include "kitchensound/pages/volume_page.h"

#include <sstream>

#include <spdlog/spdlog.h>

#include "kitchensound/timeouts.h"

#include "kitchensound/volume.h"
#include "kitchensound/renderer.h"

VolumePage::VolumePage(PAGES page, StateController& ctrl, std::shared_ptr<Volume>& vol)
        : BasePage(page, ctrl), _vol_model{}, _volume{vol} {};
VolumePage::~VolumePage() = default;

void VolumePage::handle_wheel_input(int delta) {
    if (delta != 0) {
        _vol_model.active_change_timeout = VOLUME_TIMEOUT;
        auto new_val = _volume->get_volume() + delta;
        if (new_val < 0 || new_val > 100) return;
        _volume->apply_delta(delta);
    }
}

void VolumePage::render_volume(Renderer& renderer) {
    if (_vol_model.active_change_timeout >= 0)
        --_vol_model.active_change_timeout;
    if (_vol_model.active_change_timeout < 0)
        return;

    auto vol = _volume->get_volume();

    std::ostringstream volume;
    volume << std::to_string(vol);

    //render background
    renderer.render_rect(290, 0, 30, 240, Renderer::HIGHLIGHT);

    //render volume bar
    renderer.render_rect(300, 10, 12, 200, Renderer::BACKGROUND);

    //render the actual volume
    renderer.render_rect(300, 10 + 200 - 2 * vol, 12, 2 * vol, Renderer::FOREGROUND);

    //render the volume value
    renderer.render_text(305, 225, volume.str(), Renderer::SMALL);
}

void VolumePage::enter_page(PAGES origin, void* payload) {
    //_volume.update_from_system();
    //_vol_model.new_value = _volume.get_default_volume();
    SPDLOG_INFO("Entered from -> {0}", origin);
}