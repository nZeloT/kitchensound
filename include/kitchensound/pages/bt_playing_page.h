#ifndef KITCHENSOUND_BT_PLAYING_PAGE_H
#define KITCHENSOUND_BT_PLAYING_PAGE_H

#include <memory>

#include "kitchensound/bt_controller.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/render_text.h"
#include "kitchensound/render_page.h"

class BluetoothPlayingPage : public VolumePage {
public:
    BluetoothPlayingPage(StateController* ctrl, ResourceManager& res, Volume& vol)
        : VolumePage(BT_PLAYING, ctrl, vol), _res{res}, _model{},
        _text_status{std::make_unique<RenderText>()}, _text_meta{std::make_unique<RenderText>()},
        _btc{[&](auto status, auto meta) {
            set_status(status);
            set_meta(meta);
        }} {};

    void enter_page(PAGES origin) override;
    void leave_page(PAGES destination) override;
    void handle_enter_key() override {};
    void render(Renderer &renderer) override;

private:
    void set_status(std::string const& new_status);
    void set_meta(std::string const& new_meta);

    BluetoothPlayingPageModel _model;

    ResourceManager& _res;
    BTController _btc;

    std::unique_ptr<RenderText> _text_status;
    std::unique_ptr<RenderText> _text_meta;
};

#endif //KITCHENSOUND_BT_PLAYING_PAGE_H
