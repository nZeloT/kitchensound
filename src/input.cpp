#include "kitchensound/input.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#include "kitchensound/timeouts.h"

InputSource::InputSource(const std::string &device, std::function<void (input_event &)> handler)
  : _ev{}, _handler{std::move(handler)}, _released{true}, _long_press_sent{false}, _calls_since_press{0} {
    _file_descriptor = open(device.c_str(), O_RDONLY | O_NONBLOCK);
    if (_file_descriptor < 0)
        throw std::runtime_error("Failed to open input device: " + device);
}

InputSource::~InputSource() {
    if (_file_descriptor >= 0)
        close(_file_descriptor);
};

void InputSource::check_and_handle() {
    int rc = read(_file_descriptor, &_ev, sizeof(_ev));
    if (rc < 0 && errno != EAGAIN)
        throw std::runtime_error("Error while trying to read input device! (errno: " + std::to_string(errno) + ")");

    auto call_handler = false;
    if(rc >= 0) {
        if(_ev.type == EV_KEY){
            //check if it should be a long press event
            call_handler = true;
            if(_ev.value == 1){ // Key down
                _released = false;
            }else if(_ev.value == 0) {// Key released
                _calls_since_press = 0;
                _released = true;
                _long_press_sent = false;
            }
        }
    }

    if(!_released)
        ++_calls_since_press;

    //simulate long press event
    if(!_released && !_long_press_sent && _calls_since_press > LONGPRESS_DELAY){
        _ev.value = 2;
        call_handler = true;
        _long_press_sent = true;
    }

    if(call_handler)
        _handler(_ev);
}