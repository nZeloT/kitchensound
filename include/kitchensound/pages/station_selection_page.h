#ifndef KITCHENSOUND_STATION_SELECTION_PAGE_H
#define KITCHENSOUND_STATION_SELECTION_PAGE_H

#include <utility>

#include "kitchensound/radio_station_stream.h"
#include "kitchensound/pages/selection_page.h"

class StationSelectionPage : public SelectionPage<RadioStationStream> {
public:
    StationSelectionPage(std::shared_ptr<StateController>& ctrl, std::shared_ptr<ResourceManager>& res, std::vector<RadioStationStream> streams);
    ~StationSelectionPage() override;

    void enter_page(PAGES origin, void* payload) override;

    void* leave_page(PAGES destination) override;

    void handle_wheel_input(int delta) override;
    void handle_enter_key() override;
    void render(std::unique_ptr<Renderer>& renderer) override;

    RadioStationStream* get_selected_stream();

private:
    void activate_timeout();

    struct StationBrowsingPageModel {
        int confirmed_selection;

        bool times_out;
        int remaining_time;
    } _model;
};

#endif //KITCHENSOUND_STATION_SELECTION_PAGE_H
