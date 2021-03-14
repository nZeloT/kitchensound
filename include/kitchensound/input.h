#ifndef KITCHENSOUND_INPUT_H
#define KITCHENSOUND_INPUT_H

#include <functional>
#include <string>
#include <linux/input.h>

struct InputEvent;
class Timer;
class TimerManager;

class InputSource {
public:
    InputSource(TimerManager& tm, const std::string& device, std::function<void (InputEvent &)> handler);
    ~InputSource();
    void check_and_handle();
private:
    void reset();

    std::function<void(InputEvent&)> _handler;
    int _file_descriptor;
    input_event _ev;

    bool _released;
    bool _is_long_press;
    Timer& _long_press_timer;
};

#endif //KITCHENSOUND_INPUT_H
