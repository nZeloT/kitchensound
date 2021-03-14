#include "kitchensound/input.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "kitchensound/input_event.h"
#include "kitchensound/timeouts.h"
#include "kitchensound/timer.h"
#include "kitchensound/timer_manager.h"

InputSource::InputSource(TimerManager &tm, const std::string &device, std::function<void(InputEvent &)> handler)
        : _ev{}, _handler{std::move(handler)}, _released{true}, _is_long_press{false},
          _long_press_timer{tm.request_timer(LONG_PRESS_DELAY, false, [this]() {
              this->_is_long_press = true;
          })} {
    _file_descriptor = open(device.c_str(), O_RDONLY | O_NONBLOCK);
    if (_file_descriptor < 0)
        throw std::runtime_error("Failed to open input device: " + device);
}

InputSource::~InputSource() {
    if (_file_descriptor >= 0)
        close(_file_descriptor);
};

void InputSource::reset() {
    _long_press_timer.stop();
    _is_long_press = false;
    _released = true;
}

void InputSource::check_and_handle() {
    int rc = read(_file_descriptor, &_ev, sizeof(_ev));
    if (rc < 0 && errno != EAGAIN)
        throw std::runtime_error("Error while trying to read input device! (errno: " + std::to_string(errno) + ")");

    auto call_handler = false;
    auto inev = InputEvent{_ev.value};

    if (rc >= 0) {
        if (_ev.type == EV_REL) {
            call_handler = true;
        } else if (_ev.type == EV_KEY) {
            if (_ev.value == 1) { // Key down
                _released = false;
                _long_press_timer.reset();
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
        }
    }

    if (call_handler) {
        _handler(inev);
    }
}