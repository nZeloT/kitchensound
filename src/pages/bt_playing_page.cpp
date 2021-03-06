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
        : PlayingPage(BT_PLAYING, ctrl, res, vol),
          _btc{btc} {
    set_image("", BLUETOOTH_IMAGE);
    _btc->set_metadata_status_callback([&](auto status, auto meta) {
        set_source_text(status);
        set_metadata_text(meta);
    });
};

BluetoothPlayingPage::~BluetoothPlayingPage() = default;

void BluetoothPlayingPage::enter_page(PAGES origin, void* payload) {
    BasePage::update_time();
    VolumePage::enter_page(origin, payload);
    set_source_text("Not Connected");
    set_metadata_text("");
    _btc->activate_bt();
    spdlog::info("BluetoothPlayingPage::enter_page(): Entered.");
}

void* BluetoothPlayingPage::leave_page(PAGES destination) {
    _btc->deactivate_bt();
    spdlog::info("BluetoothPlayingPage::leave_page(): Left.");
    return nullptr;
}