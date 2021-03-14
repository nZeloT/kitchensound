#ifndef KITCHENSOUND_STATION_SELECTION_PAGE_H
#define KITCHENSOUND_STATION_SELECTION_PAGE_H

#include <memory>

#include "kitchensound/radio_station_stream.h"
#include "kitchensound/pages/selection_page.h"

class MPDController;
class Timer;

class StationSelectionPage : public SelectionPage<RadioStationStream> {
public:
    StationSelectionPage(StateController &ctrl, TimerManager& tm, ResourceManager &res, std::shared_ptr<MPDController> &,
                         std::vector<RadioStationStream> streams);

    ~StationSelectionPage() override;

    void enter_page(PAGES origin, void *payload) override;

    void *leave_page(PAGES destination) override;

    void handle_wheel_input(int delta) override;

    void handle_enter_key(InputEvent&) override;

    void update(long ms_delta_update) override;

    RadioStationStream *get_selected_stream();

private:
    std::string get_text(const RadioStationStream&) override;
    void get_image(const RadioStationStream&, void**) override;

    void activate_timeout();

    std::shared_ptr<MPDController> _mpd;
    std::unique_ptr<Timer> _auto_leave_timer;

    struct StationBrowsingPageModel {
        int confirmed_selection;

        bool times_out;
    } _model;
};

#endif //KITCHENSOUND_STATION_SELECTION_PAGE_H
