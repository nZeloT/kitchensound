#include "kitchensound/pages/bt_playing_page.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/renderer.h"
#include "kitchensound/render_text.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/bt_controller.h"

#define BLUETOOTH_IMAGE "img/bluetooth.png"

BluetoothPlayingPage::BluetoothPlayingPage(ApplicationBackbone& bb,
                                           std::shared_ptr<Volume>& vol,
                                           std::shared_ptr<SongFaver>& faver,
                                           std::shared_ptr<BTController>& btc)
        : PlayingPage(PAGES::BT_PLAYING, bb, vol, faver),
          _btc{btc} {
    set_image("", BLUETOOTH_IMAGE);
    _btc->set_metadata_status_callback([this](auto status, auto song) {
        this->set_source(status);
        this->set_current_song(song);
    });
};

BluetoothPlayingPage::~BluetoothPlayingPage() = default;

void BluetoothPlayingPage::enter_page(PAGES origin, void* payload) {
    PlayingPage::enter_page(origin, payload);
    set_source("Not Connected");
    set_current_song(EMPTY_SONG);
    _btc->activate_bt();
    SPDLOG_INFO("Entered.");
}

void* BluetoothPlayingPage::leave_page(PAGES destination) {
    _btc->deactivate_bt();
    PlayingPage::leave_page(destination);
    SPDLOG_INFO("Left.");
    return nullptr;
}