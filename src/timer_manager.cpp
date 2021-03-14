#include "kitchensound/timer_manager.h"

#include "kitchensound/timer.h"

TimerManager::TimerManager() : _timers{} {};

TimerManager::~TimerManager() = default;

Timer& TimerManager::request_timer(long ms_time, bool auto_repeat, std::function<void()> trigger) {
    Timer t{ms_time, auto_repeat, std::move(trigger)};
    _timers.push_back(std::move(t));
    return _timers.back();
}

void TimerManager::update(long ms_delta_time) {
    std::for_each(begin(_timers), end(_timers), [ms_delta_time](Timer& timer) {
        timer.update(ms_delta_time);
    });
}