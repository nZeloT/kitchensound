#ifndef KITCHENSOUND_TIMER_H
#define KITCHENSOUND_TIMER_H

#include <functional>

class Timer {

public:
    ~Timer();
    double progress_percentage();

    void reset();

    void trigger_and_reset();

    void stop();

private:

    friend class TimerManager;

    Timer(long ms_time, bool auto_repeat, std::function<void()> trigger);
    void update(int ms_delta_time);

    long _ms_time;
    bool _auto_repeat;
    std::function<void()> _trigger;

    bool _is_stopped;
    int long _ms_remaining;
};

#endif //KITCHENSOUND_TIMER_H
