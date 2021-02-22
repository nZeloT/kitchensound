#ifndef KITCHENSOUND_INPUT_H
#define KITCHENSOUND_INPUT_H

#include <functional>
#include <memory>
#include <string>
#include <linux/input.h>

struct InputEvent;
class InputSource {
public:
    InputSource(const std::string& device, std::function<void (InputEvent &)> handler);
    ~InputSource();
    void check_and_handle();
private:
    void reset();

    std::function<void(InputEvent&)> _handler;
    int _file_descriptor;
    input_event _ev;

    bool _released;
    int _calls_since_press;
};

#endif //KITCHENSOUND_INPUT_H
