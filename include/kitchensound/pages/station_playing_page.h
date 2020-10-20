#ifndef KITCHENSOUND_STATION_PLAYING_PAGE_H
#define KITCHENSOUND_STATION_PLAYING_PAGE_H

#include "kitchensound/resource_manager.h"
#include "kitchensound/render_page.h"
#include "kitchensound/render_text.h"
#include "kitchensound/mpd_controller.h"

class StationPlayingPage : public VolumePage {
public:
    StationPlayingPage(StateController* ctrl, ResourceManager& res, Volume& vol) :
        VolumePage(STREAM_PLAYING, ctrl, vol),
        _res{res}, _model{},
        _text_meta{std::make_unique<RenderText>()},
        _text_status{std::make_unique<RenderText>()}
        {
            MPDController::init([&](auto new_meta) {
                set_meta_text(new_meta);
            });
        };

    void enter_page(PAGES origin) override;
    void leave_page(PAGES destination) override;
    void handle_enter_key() override;
    void render(Renderer &renderer) override;

    void set_station_playing(RadioStationStream &stream);

private:
    void set_meta_text(std::string const& new_meta) {
        if(new_meta != _text_meta->get_current_text()){
            _model.meta_changed = true;
            _model.meta_text = std::string{new_meta};
        }
    }

    void reset_model();

    StationPlayingPageModel _model;

    ResourceManager& _res;

    std::unique_ptr<RenderText> _text_status;
    std::unique_ptr<RenderText> _text_meta;
};

#endif //KITCHENSOUND_STATION_PLAYING_PAGE_H
