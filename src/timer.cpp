#include "kitchensound/timer.h"

#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/fd_registry.h"

struct Timer::Impl {
    Impl(std::unique_ptr<FdRegistry>& fdreg,  std::string  name, const long ms_time, const bool auto_repeat, std::function<void()> trigger)
        : _fdreg{fdreg}, _timerfd{-1}, _is_auto_repeat{auto_repeat}, _trigger{std::move(trigger)}, _is_stopped{true},
        _nsec_time_delay{0}, _sec_time_delay{0}, _name{std::move(name)}
    {
        _timerfd = timerfd_create(CLOCK_REALTIME, 0);
        if (_timerfd == -1)
            throw std::runtime_error{"Failed to create timer fd!"};

        _fdreg->addFd(_timerfd, [this](int fd, uint32_t epoll_events){
            this->notify();
        }, EPOLLIN);

        convert_ms(ms_time);
    }

    ~Impl() {
        _fdreg->removeFd(_timerfd);
        close(_timerfd);
    }

    void reset(long new_ms) {
        _is_stopped = false;

        if(new_ms > 0)
            convert_ms(new_ms);

        SPDLOG_DEBUG("Reset timer -> {}, {}s, {}ns", _name, _sec_time_delay, _nsec_time_delay);

        timespec now{};
        if (clock_gettime(CLOCK_REALTIME, &now) == -1)
            throw std::runtime_error{"Failed to fetch current time!"};


        itimerspec tspec{};
        tspec.it_value.tv_nsec = now.tv_nsec + _nsec_time_delay;
        tspec.it_value.tv_sec  = now.tv_sec + _sec_time_delay;

        if(tspec.it_value.tv_nsec < 0 || tspec.it_value.tv_nsec > 1000000000){
            ++tspec.it_value.tv_sec;
            tspec.it_value.tv_nsec -= 1000000000;
        }

        if(_is_auto_repeat){
            tspec.it_interval.tv_nsec = _nsec_time_delay;
            tspec.it_interval.tv_sec = _sec_time_delay;
            SPDLOG_DEBUG("Reset timer with auto repeat -> {}, {}s, {}ns", _name, tspec.it_interval.tv_sec, tspec.it_interval.tv_nsec);
        }

        if(timerfd_settime(_timerfd, TFD_TIMER_ABSTIME, &tspec, nullptr))
            throw std::runtime_error{"Failed to update timerfd!"};
    }

    void stop() {
        if(_is_stopped)
            return;
        _is_stopped = true;

        itimerspec tspec{};
        memset(&tspec, 0, sizeof(itimerspec));

        if(timerfd_settime(_timerfd, TFD_TIMER_ABSTIME, &tspec, nullptr))
            throw std::runtime_error{"Failed to stop timerfd!"};

        SPDLOG_DEBUG("Stopped -> {}", _name);
    }

    void notify() {
        SPDLOG_DEBUG("Called on timer -> {}", _name);
        //was notified now read the fd
        uint64_t timer_calls; //just read the value to know everything went smooth
        auto bytes = read(_timerfd, &timer_calls, sizeof(uint64_t));

        if(bytes == -1){
            if(errno == EAGAIN){
                SPDLOG_INFO("Received EAGAIN on timerfd. Skipping.");
                return;
            }
        }

        if(bytes != sizeof(uint64_t)) {
            throw std::runtime_error{"Failed in reading a triggered timerfd!"};
        }

        if(!_is_auto_repeat)
            _is_stopped = true; //was triggered no need to do anything else

        _trigger();
    }

    void convert_ms(long ms_time) {
        _sec_time_delay  = ms_time / 1000;
        _nsec_time_delay = (ms_time % 1000) * 1000000; // msec to nsec
    }


    uint64_t _nsec_time_delay;
    time_t _sec_time_delay;
    const bool _is_auto_repeat;
    bool _is_stopped;

    std::function<void()> _trigger;

    int _timerfd;
    std::unique_ptr<FdRegistry>& _fdreg;
    std::string _name;
};

Timer::Timer(std::unique_ptr<FdRegistry>& fdreg, std::string name, const long ms_time, const bool auto_repeat, std::function<void()> trigger)
    : _impl{std::make_unique<Impl>(fdreg, std::move(name), ms_time, auto_repeat, std::move(trigger))} {}

Timer::~Timer() = default;


void Timer::reset_timer(long new_ms) {
    _impl->reset(new_ms);
}

void Timer::notify_and_reset() {
    notify();
    reset_timer();
}

void Timer::stop() {
    _impl->stop();
}

void Timer::notify() {
    _impl->notify();
}