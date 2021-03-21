#include "kitchensound/input.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/epoll.h>

#include <spdlog/spdlog.h>

#include "kitchensound/input_event.h"
#include "kitchensound/timeouts.h"
#include "kitchensound/timer.h"
#include "kitchensound/fd_registry.h"

InputSource::InputSource(std::unique_ptr<FdRegistry> &fdr, std::string device,
                         std::function<void(InputEvent &)> handler)
        : _ev{}, _handler{std::move(handler)}, _is_long_press{false}, _fdreg{fdr},
          _device{std::move(device)},
          _long_press_timer{std::make_unique<Timer>(fdr, "Input Long Press", LONG_PRESS_DELAY, false, [this]() {
              this->_is_long_press = true;
          })} {
    _file_descriptor = open(_device.c_str(), O_RDONLY | O_NONBLOCK);
    if (_file_descriptor < 0)
        throw std::runtime_error("Failed to open input device: " + device);

    _fdreg->addFd(_file_descriptor, [this](auto fd, auto epoll_events) {
        if ((epoll_events & EPOLLIN) > 0)
            this->read_input_event();
    }, EPOLLIN);
}

InputSource::~InputSource() {
    _fdreg->removeFd(_file_descriptor);
    if (_file_descriptor >= 0)
        close(_file_descriptor);
};

void InputSource::reset() {
    _long_press_timer->stop();
    _is_long_press = false;
}

void InputSource::read_input_event() {
    int rc = read(_file_descriptor, &_ev, sizeof(_ev));
    if (rc < 0 && errno != EAGAIN)
        throw std::runtime_error("Error while trying to read input device! (errno: " + std::to_string(errno) + ")");

    auto call_handler = false;
    auto inev = InputEvent{_ev.value};

    if (rc >= 0) {
        switch (_ev.type) {
            case EV_REL:
                call_handler = true;
                break;
            case EV_KEY:
                if (_ev.value == 1) { // Key down
                    _long_press_timer->reset_timer();
                } else if (_ev.value == 0) {// Key released
                    //okay key is back up; now generate the event
                    call_handler = true;
                    //decide if long or short press event was seen
                    if (_is_long_press)
                        inev.value = INEV_KEY_LONG;
                    else
                        inev.value = INEV_KEY_SHORT;

                    reset();
                }
                break;
            default:
                return;
        }
    }

    SPDLOG_DEBUG("Read input event -> {}; rel:{}; val:{}; call:{}", _device, _ev.type == EV_REL ? true : false,
                 _ev.value, call_handler);

    if (call_handler) {
        _handler(inev);
    }
}