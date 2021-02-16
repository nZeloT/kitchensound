#ifndef KITCHENSOUND_STATION_SELECTION_PAGE_H
#define KITCHENSOUND_STATION_SELECTION_PAGE_H

#include <memory>

#include "kitchensound/radio_station_stream.h"
#include "kitchensound/pages/selection_page.h"

class MPDController;

class StationSelectionPage : public SelectionPage<RadioStationStream> {
public:
    StationSelectionPage(StateController &ctrl, ResourceManager &res, std::shared_ptr<MPDController> &,
                         std::vector<RadioStationStream> streams);

    ~StationSelectionPage() override;

    void enter_page(PAGES origin, void *payload) override;

    void *leave_page(PAGES destination) override;

    void handle_wheel_input(int delta) override;

    void handle_enter_key() override;

    void render(Renderer &renderer) override;

    RadioStationStream *get_selected_stream();

private:
    void activate_timeout();

    std::shared_ptr<MPDController> _mpd;

    struct StationBrowsingPageModel {
        int confirmed_selection;

        bool times_out;
        int remaining_time;
    } _model;
};

#endif //KITCHENSOUND_STATION_SELECTION_PAGE_H
