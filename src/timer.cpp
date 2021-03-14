#include "kitchensound/timer.h"

#include <spdlog/spdlog.h>

Timer::Timer(const long ms_time, const bool auto_repeat, std::function<void()> trigger)
    : _ms_time{ms_time}, _auto_repeat{auto_repeat}, _trigger{std::move(trigger)},
    _ms_remaining{0}, _is_stopped{false} {}

Timer::~Timer() = default;


void Timer::update(const int ms_delta_time) {
    if(_is_stopped)
        return;

    _ms_remaining -= ms_delta_time;
    if(_ms_remaining <= 0){
        _trigger();
        if(_auto_repeat) {
            _ms_remaining += _ms_time;
            if(_ms_remaining < 0) {
                //more time than expected passed; resetting to the requested time delay to prevent numerous executions
                _ms_remaining = _ms_time;
            }
        }else
            _is_stopped = true;
    }
}

void Timer::reset() {
    if(_auto_repeat){
        SPDLOG_WARN("Called for an autorepeat timer! This is not allowed!");
        return;
    }

    _ms_remaining = _ms_time;
    _is_stopped = false;
}

void Timer::trigger_and_reset() {
    _trigger();
    reset();
}

double Timer::progress_percentage() {
    return std::max(0.0d, 1 - ((_ms_remaining + 0.0d) / (_ms_time + 0.0d)));
}

void Timer::stop() {
    _is_stopped = true;
}