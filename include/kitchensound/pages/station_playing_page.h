#ifndef KITCHENSOUND_STATION_PLAYING_PAGE_H
#define KITCHENSOUND_STATION_PLAYING_PAGE_H

#include "kitchensound/radio_station_stream.h"
#include "kitchensound/pages/volume_page.h"

class ResourceManager;
class RenderText;

class StationPlayingPage : public VolumePage {
public:
    StationPlayingPage(std::shared_ptr<StateController> &ctrl, std::shared_ptr<ResourceManager> &res,
                       std::shared_ptr<Volume> &vol, RadioStationStream* initial_station);
    ~StationPlayingPage() override;

    void enter_page(PAGES origin, void* payload) override;

    void* leave_page(PAGES destination) override;

    void handle_enter_key() override;

    void render(std::unique_ptr<Renderer> &renderer) override;

private:

    void set_station_playing(RadioStationStream* stream);

    void set_meta_text(std::string const &new_meta);

    struct StationPlayingPageModel {
        StationPlayingPageModel() : station{},
                                    station_changed{true}, meta_changed{true}, meta_text{} {};

        RadioStationStream station;

        bool station_changed;

        bool meta_changed;
        std::string meta_text;
    } _model;

    std::shared_ptr<ResourceManager>& _res;

    std::unique_ptr<RenderText> _text_status;
    std::unique_ptr<RenderText> _text_meta;
};

#endif //KITCHENSOUND_STATION_PLAYING_PAGE_H
