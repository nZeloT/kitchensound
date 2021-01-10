#ifndef KITCHENSOUND_STANDBY_H
#define KITCHENSOUND_STANDBY_H

#include <ctime>

class Configuration;

class StandBy {
public:
    explicit StandBy(Configuration &conf);

    void arm() { _armed = true; };
    void disarm() { _armed = false; };

    void update_time();

    void reset_standby_cooldown();

    bool is_standby_active();

private:
    std::tm* _current_time;
    bool _enabled;
    bool _armed;

    int _start_hour;
    int _start_min;
    int _end_hour;
    int _end_min;

    int _cooldown_timer;
};

#endif //KITCHENSOUND_STANDBY_H
