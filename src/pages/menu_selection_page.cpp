#include "kitchensound/pages/menu_selection_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/input_event.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/application_backbone.h"

const std::vector<MenuModel> MENUS = {
        {"Radio", STREAM_PLAYING, "img/radio.png"},
        {"Bluetooth", BT_PLAYING, "img/bluetooth.png"},
        {"Multiroom", SNAPCAST_PLAYING, "img/speaker_group.png"},
        {"Optionen", OPTIONS, "img/gears.png"}
};

MenuSelectionPage::MenuSelectionPage(ApplicationBackbone& bb)
        : SelectionPage<MenuModel>(MENU_SELECTION, bb, MENUS) {
    load_images();
}

MenuSelectionPage::~MenuSelectionPage() = default;

std::string MenuSelectionPage::get_text(const MenuModel &m) {
    return m.name;
}

void MenuSelectionPage::get_image(const MenuModel &m, void ** image_data_ptr) {
    *image_data_ptr = _bb.res->get_static(m.static_image);
}

void MenuSelectionPage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT) {
        auto dest = _sp_model.data[_sp_model.selected].ref_page;
        _bb.ctrl->trigger_transition(_page, dest);
        SPDLOG_INFO("Transitioning from menu -> {0}", dest);
    }
}

void MenuSelectionPage::enter_page(PAGES orig, void* payload) {
    SelectionPage<MenuModel>::enter_page(orig, payload);
    SPDLOG_INFO("Enter from -> {0}", orig);
}

void* MenuSelectionPage::leave_page(PAGES dest) {
    SelectionPage<MenuModel>::leave_page(dest);
    SPDLOG_INFO("Leaving to -> {0}", dest);
    return nullptr;
}