#include "kitchensound/pages/menu_selection_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/input_event.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"

const std::vector<MenuModel> MENUS = {
        {"Radio", STREAM_PLAYING, "img/radio.png"},
        {"Bluetooth", BT_PLAYING, "img/bluetooth.png"},
        {"Optionen", OPTIONS, "img/gears.png"}
};

MenuSelectionPage::MenuSelectionPage(StateController& ctrl, ResourceManager& res)
        : SelectionPage<MenuModel>(MENU_SELECTION, ctrl, res, MENUS) {
    load_images();
}

MenuSelectionPage::~MenuSelectionPage() = default;

std::string MenuSelectionPage::get_text(const MenuModel &m) {
    return m.name;
}

void MenuSelectionPage::get_image(const MenuModel &m, void ** image_data_ptr) {
    *image_data_ptr = _res.get_static(m.static_image);
}

void MenuSelectionPage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_DOWN) {
        auto dest = _sp_model.data[_sp_model.selected].ref_page;
        _state.trigger_transition(_page, dest);
        spdlog::info("MenuSelectionPage::handle_enter_key(): transitioning from Mode Selection to {0}", dest);
    }
}

void MenuSelectionPage::enter_page(PAGES orig, void* payload) {
    BasePage::enter_page(orig, payload);
    spdlog::info("MenuSelectionPage::enter_page(): from origin {0}", orig);
}

void* MenuSelectionPage::leave_page(PAGES dest) {
    spdlog::info("MenuSelectionPage::leave_page(): to destination {0}", dest);
    return nullptr;
}