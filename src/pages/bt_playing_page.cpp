#include "kitchensound/pages/bt_playing_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/renderer.h"
#include "kitchensound/render_text.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/bt_controller.h"

#define BLUETOOTH_IMAGE "img/bluetooth.png"

BluetoothPlayingPage::BluetoothPlayingPage(StateController &ctrl,
                                           ResourceManager &res,
                                           std::shared_ptr<Volume>& vol,
                                           std::shared_ptr<BTController>& btc)
        : VolumePage(BT_PLAYING, ctrl, vol), _res{res}, _model{},
          _text_status{std::make_unique<RenderText>()}, _text_meta{std::make_unique<RenderText>()},
          _btc{btc} {
    _btc->set_metadata_status_callback([&](auto status, auto meta) {
        set_status(status);
        set_meta(meta);
    });
};

BluetoothPlayingPage::~BluetoothPlayingPage() = default;

void BluetoothPlayingPage::render(Renderer& renderer) {
    this->render_time(renderer);

    //1. render the artwork
    auto image = reinterpret_cast<SDL_Surface *>(_res.get_static(BLUETOOTH_IMAGE));
    renderer.render_image(image, 96, 36, 128, 128);

    //2. render the status message
    if (_model.status_changed) {
        _model.status_changed = false;
        _text_status->change_text(renderer, _model.status, 160, 180);
    }

    _text_status->update_and_render(renderer);

    //3. render the metadata
    if (_model.meta_changed) {
        _model.meta_changed = false;
        _text_meta->change_text(renderer, _model.meta, 160, 210);
    }

    _text_meta->update_and_render(renderer);

    this->render_volume(renderer);
}

void BluetoothPlayingPage::enter_page(PAGES origin, void* payload) {
    BasePage::update_time();
    VolumePage::enter_page(origin, payload);
    _model.meta_changed = true;
    _model.meta = "";
    _model.status_changed = true;
    _model.status = "Not Connected";
    _btc->activate_bt();
    spdlog::info("BluetoothPlayingPage::enter_page(): Entered.");
}

void* BluetoothPlayingPage::leave_page(PAGES destination) {
    _btc->deactivate_bt();
    spdlog::info("BluetoothPlayingPage::leave_page(): Left.");
    return nullptr;
}

void BluetoothPlayingPage::set_meta(const std::string &new_meta) {
    _model.meta = std::string{new_meta};
    _model.meta_changed = true;
}

void BluetoothPlayingPage::set_status(const std::string &new_status) {
    _model.status = std::string{new_status};
    _model.status_changed = true;
}