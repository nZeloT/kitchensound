#ifndef KITCHENSOUND_STATION_PLAYING_PAGE_H
#define KITCHENSOUND_STATION_PLAYING_PAGE_H

#include "kitchensound/radio_station_stream.h"
#include "kitchensound/pages/playing_page.h"

class MPDController;

class StationPlayingPage : public PlayingPage {
public:
    StationPlayingPage(StateController &, TimerManager&, ResourceManager &, std::shared_ptr<Volume> &,
                       std::shared_ptr<MPDController> &, RadioStationStream *);

    ~StationPlayingPage() override;

    void enter_page(PAGES origin, void *payload) override;

    void *leave_page(PAGES destination) override;

    void handle_enter_key(InputEvent&) override;

private:

    void set_station_playing(RadioStationStream *stream);

    struct StationPlayingPageModel {
        StationPlayingPageModel() : station{} {};
        RadioStationStream station;
    } _model;

    std::shared_ptr<MPDController> _mpd;
};

#endif //KITCHENSOUND_STATION_PLAYING_PAGE_H
