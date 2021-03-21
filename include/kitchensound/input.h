#ifndef KITCHENSOUND_INPUT_H
#define KITCHENSOUND_INPUT_H

#include <linux/input.h>

#include <functional>
#include <string>
#include <memory>

struct InputEvent;
class Timer;
class FdRegistry;

class InputSource {
public:
    InputSource(std::unique_ptr<FdRegistry>&, std::string, std::function<void (InputEvent &)>);
    ~InputSource();
private:
    void reset();
    void read_input_event();

    std::function<void(InputEvent&)> _handler;
    int _file_descriptor;
    input_event _ev;

    bool _is_long_press;
    std::unique_ptr<Timer> _long_press_timer;
    std::unique_ptr<FdRegistry>& _fdreg;
    std::string _device;
};

#endif //KITCHENSOUND_INPUT_H
