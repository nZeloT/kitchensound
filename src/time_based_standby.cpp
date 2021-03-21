#include "kitchensound/time_based_standby.h"

#include "kitchensound/timeouts.h"
#include "kitchensound/timer.h"

TimeBasedStandby::TimeBasedStandby(Configuration::DisplayStandbyConfig c, std::unique_ptr<FdRegistry>& fdr)
        : _interval_a{}, _interval_b{}, _armed{}, _currently_active{false}, _current_time{nullptr},
        _change_callback{[](auto b){}},
        _update_timer{std::make_unique<Timer>(fdr, "Standby Clock Update", CLOCK_UPDATE_DELAY, true, [this]() {
            this->update_time();
            auto old_state = this->_currently_active;
            this->_currently_active = this->is_standby_active();
            if(old_state != this->_currently_active)
                this->_change_callback(this->_currently_active);
        })}{
    update_time();

    _enabled = c.enabled;

    //does the given interval go over midnight?
    _interval_a.active = true;
    if (c.start_hour > c.end_hour) {
        //check only fails if the display should stay off for 23 hours and only become active for one hour

        //split the interval around midnight
        _interval_a.start_hour = c.start_hour;
        _interval_a.start_min = c.start_minute;
        _interval_a.end_hour = 23;
        _interval_a.end_min = 59;

        _interval_b.active = true;
        _interval_b.start_hour = 0;
        _interval_b.start_min = 0;
        _interval_b.end_hour = c.end_hour;
        _interval_b.end_min = c.end_minute;

    } else {
        _interval_a.start_hour = c.start_hour;
        _interval_a.start_min = c.start_minute;
        _interval_a.end_hour = c.end_hour;
        _interval_a.end_min = c.end_minute;
    }

    _currently_active = is_standby_active();
}

TimeBasedStandby::~TimeBasedStandby() = default;

void TimeBasedStandby::update_time() {
    auto now = std::time(nullptr);
    _current_time = std::localtime(&now);
}

bool TimeBasedStandby::is_standby_active() {
    return _enabled
           && _armed
           && _current_time != nullptr

           && (
                   (_interval_b.active &&
                    (_interval_a.is_active(_current_time) || _interval_b.is_active(_current_time)))
                   || _interval_a.is_active(_current_time)
           );
}

void TimeBasedStandby::set_change_callback(std::function<void(bool)> cb) {
    _change_callback = std::move(cb);
}