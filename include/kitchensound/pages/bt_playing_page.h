#ifndef KITCHENSOUND_BT_PLAYING_PAGE_H
#define KITCHENSOUND_BT_PLAYING_PAGE_H

#include <memory>

#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/playing_page.h"

struct ApplicationBackbone;
class BTController;

class BluetoothPlayingPage : public PlayingPage {
public:
    BluetoothPlayingPage(ApplicationBackbone&,
                         std::shared_ptr<Volume>&,
                         std::shared_ptr<BTController>&);

    ~BluetoothPlayingPage() override;

    void enter_page(PAGES, void*) override;

    void* leave_page(PAGES) override;

    void handle_enter_key(InputEvent&) override {};

private:
    std::shared_ptr<BTController> _btc;
};

#endif //KITCHENSOUND_BT_PLAYING_PAGE_H
