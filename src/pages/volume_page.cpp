#include "kitchensound/pages/volume_page.h"

#include <sstream>

#include <spdlog/spdlog.h>

#include "kitchensound/timeouts.h"

#include "kitchensound/volume.h"
#include "kitchensound/renderer.h"

VolumePage::VolumePage(PAGES page, std::shared_ptr<StateController> &ctrl, std::shared_ptr<Volume> &vol)
        : BasePage(page, ctrl), _volume{vol}, _vol_model{} {};
VolumePage::~VolumePage() = default;

void VolumePage::handle_wheel_input(int delta) {
    if (delta != 0) {
        _vol_model.active_change_timeout = VOLUME_TIMEOUT;
        auto new_val = _volume->get_volume() + delta;
        if (new_val < 0 || new_val > 100) return;
        _volume->apply_delta(delta);
    }
}

void VolumePage::render_volume(std::unique_ptr<Renderer>& renderer) {
    if (_vol_model.active_change_timeout >= 0)
        --_vol_model.active_change_timeout;
    if (_vol_model.active_change_timeout < 0)
        return;

    auto vol = _volume->get_volume();

    std::ostringstream volume;
    volume << std::to_string(vol);

    //render background
    renderer->render_highlight(290, 0, 30, 240);

    //render volume bar
    renderer->render_background(300, 10, 12, 200);

    //render the actual volume
    renderer->render_foreground(300, 10 + 200 - 2 * vol, 12, 2 * vol);

    //render the volume value
    renderer->render_text_small(305, 225, volume.str());
}

void VolumePage::enter_page(PAGES origin, void* payload) {
    //_volume.update_from_system();
    //_vol_model.new_value = _volume.get_volume();
    spdlog::info("VolumePage::enter_page(): Entered.");
}