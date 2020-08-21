#ifndef KITCHENSOUND_STATION_BROWSING_PAGE_H
#define KITCHENSOUND_STATION_BROWSING_PAGE_H

#include <utility>

#include "kitchensound/render_page.h"
#include "kitchensound/resource_manager.h"

class StationBrowsingPage : public BasePage {
public:
    StationBrowsingPage(StateController* ctrl, ResourceManager& res, std::vector<RadioStationStream> streams) : BasePage(STREAM_BROWSING, ctrl),
        _res{res}, _model{} {
        _model.stations = std::move(streams);
        _model.limit    = _model.stations.size();
    };
    void enter_page(PAGES origin) override {
        this->update_time();
        _model.times_out = false;
        _model.remaining_time = -1;
        if(origin == STREAM_PLAYING)
            activate_timeout();
    }
    void leave_page(PAGES destination) override {};
    void handle_wheel_input(int delta) override;
    void handle_enter_key() override;
    void render(Renderer &renderer) override;

    RadioStationStream get_selected_stream() { return _model.stations[_model.confirmed_selection]; };

private:
    void activate_timeout();

    ResourceManager& _res;

    StationBrowsingPageModel _model;
};

#endif //KITCHENSOUND_STATION_BROWSING_PAGE_H
