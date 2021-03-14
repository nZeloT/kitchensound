#ifndef KITCHENSOUND_TIMER_MANAGER_H
#define KITCHENSOUND_TIMER_MANAGER_H

#include <vector>
#include <functional>

class Timer;

class TimerManager {
public:
    TimerManager();
    ~TimerManager();

    Timer& request_timer(long ms_time, bool auto_repeat, std::function<void()> trigger);

    void update(long ms_delta_time);

private:
    std::vector<Timer> _timers;
};

#endif //KITCHENSOUND_TIMER_MANAGER_H
