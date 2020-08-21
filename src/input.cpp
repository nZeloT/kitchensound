#include "kitchensound/input.h"

#include <fcntl.h>
#include <unistd.h>
#include <iostream>

InputSource::InputSource(const std::string &device, std::function<void (input_event &)> handler)
  : _ev{}, _handler{std::move(handler)} {
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

    if(rc >= 0) {
        _handler(_ev);
    }
}