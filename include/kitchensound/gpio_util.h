#ifndef KITCHENSOUND_GPIO_UTIL_H
#define KITCHENSOUND_GPIO_UTIL_H

#include <memory>

struct gpiod_chip;
struct gpiod_line;

class GpioUtil {
public:
    GpioUtil(int, int);

    ~GpioUtil();

    void turn_on_display();

    void turn_off_display();

    void turn_on_amplifier();

    void turn_off_amplifier();
private:

    gpiod_chip *_chip;
    gpiod_line *_line_display;
    gpiod_line *_line_amplifier;
};


#endif //KITCHENSOUND_GPIO_UTIL_H
