#ifndef KITCHENSOUND_TIMER_H
#define KITCHENSOUND_TIMER_H

#include <functional>

class Timer {

public:
    Timer(long ms_time, bool auto_repeat, std::function<void()> trigger);
    ~Timer();

    void update(int ms_delta_time);

    double progress_percentage();

    void reset();

private:
    const long _ms_time;
    const bool _auto_repeat;
    std::function<void()> _trigger;

    int long _ms_remaining;
    bool _was_triggered;
};

#endif //KITCHENSOUND_TIMER_H
