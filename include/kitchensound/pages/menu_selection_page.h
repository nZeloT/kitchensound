#ifndef KITCHENSOUND_MENU_SELECTION_PAGE_H
#define KITCHENSOUND_MENU_SELECTION_PAGE_H

#include "kitchensound/pages/selection_page.h"

class MenuSelectionPage : public SelectionPage<ModeModel> {
public:
    MenuSelectionPage(StateController* ctrl, ResourceManager& res);

    void handle_enter_key() override;
    void enter_page(PAGES orig) override;
    void leave_page(PAGES dest) override;
};

#endif //KITCHENSOUND_MENU_SELECTION_PAGE_H
