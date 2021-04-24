#ifndef KITCHENSOUND_STATION_PLAYING_PAGE_H
#define KITCHENSOUND_STATION_PLAYING_PAGE_H

#include "kitchensound/radio_station_stream.h"
#include "kitchensound/pages/playing_page.h"

class MPDController;
struct ApplicationBackbone;

class StationPlayingPage : public PlayingPage {
public:
    StationPlayingPage(ApplicationBackbone&, std::shared_ptr<Volume> &, std::shared_ptr<SongFaver>&,
                       std::shared_ptr<MPDController> &, RadioStationStream *);

    ~StationPlayingPage() override;

    void enter_page(PAGES, void *) override;

    void *leave_page(PAGES) override;

    void handle_enter_key(InputEvent&) override;

private:

    void set_station_playing(RadioStationStream *);

    struct StationPlayingPageModel {
        StationPlayingPageModel() : station{} {};
        RadioStationStream station;
    } _model;

    std::shared_ptr<MPDController> _mpd;
};

#endif //KITCHENSOUND_STATION_PLAYING_PAGE_H
