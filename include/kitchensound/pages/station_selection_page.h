#ifndef KITCHENSOUND_STATION_SELECTION_PAGE_H
#define KITCHENSOUND_STATION_SELECTION_PAGE_H

#include <memory>

#include "kitchensound/radio_station_stream.h"
#include "kitchensound/pages/selection_page.h"

struct ApplicationBackbone;
class MPDController;
class Timer;

class StationSelectionPage : public SelectionPage<RadioStationStream> {
public:
    StationSelectionPage(ApplicationBackbone&, std::shared_ptr<MPDController> &,
                         std::vector<RadioStationStream>);

    ~StationSelectionPage() override;

    void enter_page(PAGES, void *) override;

    void *leave_page(PAGES) override;

    void handle_wheel_input(int) override;

    void handle_enter_key(InputEvent&) override;

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
