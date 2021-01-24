#include "kitchensound/pages/menu_selection_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/model.h"
#include "kitchensound/state_controller.h"

MenuSelectionPage::MenuSelectionPage(StateController *ctrl, ResourceManager &res)
        : SelectionPage<ModeModel>(MENU_SELECTION, ctrl, res, MENUS,
                                   [](ResourceManager &r, const ModeModel &m) {
                                       void *image_ptr = r.get_static(m.static_image);
                                       if (image_ptr == nullptr)
                                           throw std::runtime_error{
                                                   "MenuSelectionPage::C-Tor(): Failed to load static image!"};
                                       return image_ptr;
                                   },
                                   [](const ModeModel &m) {
                                       return m.name;
                                   }) {}

void MenuSelectionPage::handle_enter_key() {
    auto dest = _sp_model.data[_sp_model.selected].ref_page;
    _state->trigger_transition(_page, dest);
    spdlog::info("MenuSelectionPage::handle_enter_key(): transitioning from Mode Selection to {0}", dest);
}

void MenuSelectionPage::enter_page(PAGES orig) {
    spdlog::info("MenuSelectionPage::enter_page(): from origin {0}", orig);
}

void MenuSelectionPage::leave_page(PAGES dest) {
    spdlog::info("MenuSelectionPage::leave_page(): to destination {0}", dest);
}