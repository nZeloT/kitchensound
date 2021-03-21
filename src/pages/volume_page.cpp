#include "kitchensound/pages/volume_page.h"

#include "kitchensound/timeouts.h"

#include "kitchensound/volume.h"
#include "kitchensound/renderer.h"
#include "kitchensound/timer.h"
#include "kitchensound/application_backbone.h"

VolumePage::VolumePage(PAGES page, ApplicationBackbone& bb, std::shared_ptr<Volume>& vol)
        : BasePage(page, bb), _volume_bar_visible{false}, _volume{vol},
          _volume_bar_timeout{std::make_unique<Timer>(bb.fdreg, "VolumePage Volume Bar Hide", VOLUME_TIMEOUT, false, [this](){
              this->_volume_bar_visible = false;
          })}
{};
VolumePage::~VolumePage() = default;

void VolumePage::handle_wheel_input(int delta) {
    if (delta != 0) {
        _volume_bar_visible = true;
        _volume_bar_timeout->reset_timer();
        auto new_val = _volume->get_volume() + delta;
        if (new_val < 0 || new_val > 100) return;
        _volume->apply_delta(delta);
    }
}

void* VolumePage::leave_page(PAGES dest) {
    _volume_bar_timeout->stop();
    return BasePage::leave_page(dest);
}

void VolumePage::render_volume() {
    if (!_volume_bar_visible)
        return;

    auto vol = _volume->get_volume();

    std::ostringstream volume;
    volume << std::to_string(vol);

    auto& renderer = _bb.rend;

    //render background
    renderer->render_rect(290, 0, 30, 240, Renderer::HIGHLIGHT);

    //render volume bar
    renderer->render_rect(300, 10, 12, 200, Renderer::BACKGROUND);

    //render the actual volume
    renderer->render_rect(300, 10 + 200 - 2 * vol, 12, 2 * vol, Renderer::FOREGROUND);

    //render the volume value
    renderer->render_text(305, 225, volume.str(), Renderer::SMALL);
}