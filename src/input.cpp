#include "kitchensound/input.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "kitchensound/input_event.h"
#include "kitchensound/timeouts.h"

InputSource::InputSource(const std::string &device, std::function<void (InputEvent&)> handler)
  : _ev{}, _handler{std::move(handler)}, _released{true}, _calls_since_press{0} {
    _file_descriptor = open(device.c_str(), O_RDONLY | O_NONBLOCK);
    if (_file_descriptor < 0)
        throw std::runtime_error("Failed to open input device: " + device);
}

InputSource::~InputSource() {
    if (_file_descriptor >= 0)
        close(_file_descriptor);
};

void InputSource::reset() {
    _calls_since_press = 0;
    _released = true;
}

void InputSource::check_and_handle() {
    int rc = read(_file_descriptor, &_ev, sizeof(_ev));
    if (rc < 0 && errno != EAGAIN)
        throw std::runtime_error("Error while trying to read input device! (errno: " + std::to_string(errno) + ")");

    auto call_handler = false;
    auto inev = InputEvent{_ev.value};

    if(rc >= 0) {
        if(_ev.type == EV_REL) {
            call_handler = true;
        }else if(_ev.type == EV_KEY){
            if(_ev.value == 1){ // Key down
                _released = false;
            }else if(_ev.value == 0) {// Key released
                //okay key is back up; now generate the event
                call_handler = true;
                //decide if long or short press event was seen
                if(_calls_since_press > LONGPRESS_DELAY)
                    inev.value = INEV_KEY_LONG;
                else
                    inev.value = INEV_KEY_SHORT;

                reset();
            }
        }
    }

    if(!_released) {
        ++_calls_since_press;
    }

    if(call_handler){
        _handler(inev);
    }
}