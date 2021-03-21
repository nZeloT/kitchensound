#ifndef KITCHENSOUND_TIMER_H
#define KITCHENSOUND_TIMER_H

#include <memory>
#include <functional>

class FdRegistry;

class Timer {

public:
    Timer(std::unique_ptr<FdRegistry>&, std::string, long, bool, std::function<void()>);

    ~Timer();

    void reset_timer(long = 0);

    void notify_and_reset();

    void stop();

    void notify();

private:

    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif //KITCHENSOUND_TIMER_H
