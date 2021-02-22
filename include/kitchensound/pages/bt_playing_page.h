#ifndef KITCHENSOUND_BT_PLAYING_PAGE_H
#define KITCHENSOUND_BT_PLAYING_PAGE_H

#include <memory>

#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/playing_page.h"

class BTController;

class BluetoothPlayingPage : public PlayingPage {
public:
    BluetoothPlayingPage(StateController &ctrl,
                         ResourceManager &res,
                         std::shared_ptr<Volume>& vol,
                         std::shared_ptr<BTController>& btc);

    ~BluetoothPlayingPage() override;

    void enter_page(PAGES origin, void* payload) override;

    void* leave_page(PAGES destination) override;

    void handle_enter_key(InputEvent&) override {};

private:
    std::shared_ptr<BTController> _btc;
};

#endif //KITCHENSOUND_BT_PLAYING_PAGE_H
