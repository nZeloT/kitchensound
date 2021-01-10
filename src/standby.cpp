#include "kitchensound/standby.h"

#include "kitchensound/config.h"

#define COOLDOWN_ITERATIONS 3


StandBy::StandBy(Configuration &conf) {
    update_time();

    auto c = conf.get_display_standby();
    _enabled = c.enabled;
    _start_hour = c.start_hour;
    _start_min = c.start_minute;

    _end_hour = c.end_hour;
    _end_min = c.end_minute;

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

           && _current_time->tm_hour >= _start_hour
           && _current_time->tm_min >= _start_min
           && _current_time->tm_hour <= _end_hour
           && _current_time->tm_min <= _end_min

           && _cooldown_timer == 0;
}