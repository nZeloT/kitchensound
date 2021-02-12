#ifndef KITCHENSOUND_BT_PLAYING_PAGE_H
#define KITCHENSOUND_BT_PLAYING_PAGE_H

#include <memory>

#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/volume_page.h"

class BTController;

class RenderText;

class ResourceManager;

class BluetoothPlayingPage : public VolumePage {
public:
    BluetoothPlayingPage(StateController &ctrl, Volume& vol,
                         ResourceManager &res);

    ~BluetoothPlayingPage() override;

    void enter_page(PAGES origin, void* payload) override;

    void* leave_page(PAGES destination) override;

    void handle_enter_key() override {};

    void render(Renderer& renderer) override;

private:
    void set_status(std::string const &new_status);

    void set_meta(std::string const &new_meta);

    struct BluetoothPlayingPageModel {
        BluetoothPlayingPageModel() : status_changed{true},
                                      status{}, meta_changed{true}, meta{} {};

        bool status_changed;
        std::string status;

        bool meta_changed;
        std::string meta;
    } _model;

    ResourceManager& _res;

    std::unique_ptr<BTController> _btc;

    std::unique_ptr<RenderText> _text_status;
    std::unique_ptr<RenderText> _text_meta;
};

#endif //KITCHENSOUND_BT_PLAYING_PAGE_H
