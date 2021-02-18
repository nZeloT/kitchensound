#ifndef KITCHENSOUND_INPUT_H
#define KITCHENSOUND_INPUT_H

#include <functional>
#include <memory>
#include <string>
#include <linux/input.h>

class InputSource {
public:
    InputSource(const std::string& device, std::function<void (input_event &)> handler);
    ~InputSource();
    void check_and_handle();
private:
    std::function<void(input_event&)> _handler;
    int _file_descriptor;
    input_event _ev;

    bool _released;
    bool _long_press_sent;
    int _calls_since_press;
};

#endif //KITCHENSOUND_INPUT_H
