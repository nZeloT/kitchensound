#ifndef KITCHENSOUND_TIME_BASED_STANDBY_H
#define KITCHENSOUND_TIME_BASED_STANDBY_H

#include <functional>
#include <chrono>

#include "kitchensound/config.h"

class FdRegistry;
class Timer;

class TimeBasedStandby {
public:
    explicit TimeBasedStandby(Configuration::DisplayStandbyConfig, std::unique_ptr<FdRegistry>&);
    ~TimeBasedStandby();

    void arm();

    void disarm();

    void update_time();

    bool is_standby_active();

    void set_change_callback(std::function<void(bool)>);

private:
    void update_state();

    std::tm* _current_time;
    bool _enabled;
    bool _armed;

    struct TimeInterval {
        bool active = false;
        int start_hour = 0;
        int start_min = 0;
        int end_hour = 0;
        int end_min = 0;

        bool is_active(std::tm *time) const {
            return !active
                   || (
                           (time->tm_hour > start_hour || (time->tm_hour == start_hour && time->tm_min >= start_min))
                           && (time->tm_hour < end_hour || (time->tm_hour == end_hour && time->tm_min <= end_min))
                   );
        }
    };

    TimeInterval _interval_a;
    TimeInterval _interval_b;

    bool _currently_active;
    std::function<void(bool)> _change_callback;

    std::unique_ptr<Timer> _update_timer;
};

#endif //KITCHENSOUND_TIME_BASED_STANDBY_H
