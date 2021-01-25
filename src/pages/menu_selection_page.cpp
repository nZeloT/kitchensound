#include "kitchensound/pages/menu_selection_page.h"

#include <spdlog/spdlog.h>

#include <utility>

#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"

const std::vector<MenuModel> MENUS = {
        {"Radio", STREAM_PLAYING, "img/radio.png"},
        {"Bluetooth", BT_PLAYING, "img/bluetooth.png"},
        {"Optionen", OPTIONS, "img/gears.png"}
};

MenuSelectionPage::MenuSelectionPage(std::shared_ptr<StateController>& ctrl, std::shared_ptr<ResourceManager>& res)
        : SelectionPage<MenuModel>(MENU_SELECTION, ctrl, res, MENUS,
                                   [](std::shared_ptr<ResourceManager>& r, const MenuModel &m) {
                                       void *image_ptr = r->get_static(m.static_image);
                                       if (image_ptr == nullptr)
                                           throw std::runtime_error{
                                                   "MenuSelectionPage::C-Tor(): Failed to load static image!"};
                                       return image_ptr;
                                   },
                                   [](const MenuModel &m) {
                                       return m.name;
                                   }) {}

MenuSelectionPage::~MenuSelectionPage() = default;

void MenuSelectionPage::handle_enter_key() {
    auto dest = _sp_model.data[_sp_model.selected].ref_page;
    _state->trigger_transition(_page, dest);
    spdlog::info("MenuSelectionPage::handle_enter_key(): transitioning from Mode Selection to {0}", dest);
}

void MenuSelectionPage::enter_page(PAGES orig, void* payload) {
    spdlog::info("MenuSelectionPage::enter_page(): from origin {0}", orig);
}

void* MenuSelectionPage::leave_page(PAGES dest) {
    spdlog::info("MenuSelectionPage::leave_page(): to destination {0}", dest);
    return nullptr;
}