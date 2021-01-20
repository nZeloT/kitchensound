#include "kitchensound/standby.h"

#include "kitchensound/config.h"

#define COOLDOWN_ITERATIONS 3


StandBy::StandBy(Configuration &conf)
        : _interval_a{}, _interval_b{}, _armed{}, _current_time{nullptr} {
    update_time();

    auto c = conf.get_display_standby();
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

    _cooldown_timer = COOLDOWN_ITERATIONS;
}

void StandBy::update_time() {
    auto now = std::time(nullptr);
    _current_time = std::localtime(&now);
    if (_armed && _cooldown_timer > 0)
        --_cooldown_timer;

}

void StandBy::reset_standby_cooldown() {
    _cooldown_timer = COOLDOWN_ITERATIONS;
}

bool StandBy::is_standby_active() {
    return _enabled
           && _armed
           && _current_time != nullptr

           && (
                   (_interval_b.active &&
                    (_interval_a.is_active(_current_time) || _interval_b.is_active(_current_time)))
                   || _interval_a.is_active(_current_time)
           )

           && _cooldown_timer == 0;
}