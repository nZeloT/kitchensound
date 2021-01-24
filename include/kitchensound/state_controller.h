#ifndef KITCHENSOUND_STATE_CONTROLLER_H
#define KITCHENSOUND_STATE_CONTROLLER_H

#include <memory>
#include <queue>

#include "kitchensound/model.h"
#include "kitchensound/config.h"
#include "kitchensound/time_based_standby.h"
#include "kitchensound/renderer.h"
#include "kitchensound/render_page.h"
#include "kitchensound/pages/bt_playing_page.h"
#include "kitchensound/pages/station_selection_page.h"
#include "kitchensound/pages/station_playing_page.h"
#include "kitchensound/pages/menu_selection_page.h"
#include "kitchensound/pages/options_page.h"

class StateController {
public:
    StateController(Configuration &conf, ResourceManager &res, Renderer &renderer);

    ~StateController();

    void update(bool time);

    void render();

    void trigger_transition(PAGES origin, PAGES destination);

    void react_wheel_input(int delta);

    void react_menu_change();

    void react_confirm();

    void react_power_change();

    bool is_standby_active();

private:
    void update_time();

    void process_transition();

    void transition_select_next_page();

    enum TRANSITION_STATE {
        NONE,
        LEAVING_LOADING,
        ENTERING,
        LEAVING,
        ENTER_LOADING
    };

    Renderer &_renderer;
    BasePage *_active_page;
    BasePage *_previous_page;
    BasePage *_next_page;
    TRANSITION_STATE _transitions;
    PAGES _transition_origin;
    PAGES _transition_destination;

    LoadingPage _loading;
    InactivePage _inactive;
    MenuSelectionPage _mode_selection;
    StationSelectionPage _stream_selection;
    StationPlayingPage _stream_playing;
    BluetoothPlayingPage _bt_playing;
    OptionsPage _options_page;

    Volume _volume;
    TimeBasedStandby _standby;
};

#endif //KITCHENSOUND_STATE_CONTROLLER_H
