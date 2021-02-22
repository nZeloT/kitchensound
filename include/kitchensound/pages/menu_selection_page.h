#ifndef KITCHENSOUND_MENU_SELECTION_PAGE_H
#define KITCHENSOUND_MENU_SELECTION_PAGE_H

#include "kitchensound/pages/selection_page.h"
#include "kitchensound/menu_model.h"

class MenuSelectionPage : public SelectionPage<MenuModel> {
public:
    MenuSelectionPage(StateController& ctrl, ResourceManager& res);
    ~MenuSelectionPage() override;
    void handle_enter_key(InputEvent&) override;
    void enter_page(PAGES orig, void* payload) override;
    void* leave_page(PAGES dest) override;

private:
    std::string get_text(const MenuModel&) override;
    void get_image(const MenuModel&, void**) override;
};

#endif //KITCHENSOUND_MENU_SELECTION_PAGE_H
