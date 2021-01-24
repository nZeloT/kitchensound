#ifndef KITCHENSOUND_TIME_BASED_STANDBY_H
#define KITCHENSOUND_TIME_BASED_STANDBY_H

#include <ctime>

class Configuration;

class TimeBasedStandby {
public:
    explicit TimeBasedStandby(Configuration &conf);

    void arm() { _armed = true; };

    void disarm() { _armed = false; };

    void update_time();

    void reset_standby_cooldown();

    bool is_standby_active();

private:
    std::tm *_current_time;
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

    int _cooldown_timer;
};

#endif //KITCHENSOUND_TIME_BASED_STANDBY_H
