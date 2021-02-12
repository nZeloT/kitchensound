#include "kitchensound/gpio_util.h"

#include <gpiod.h>

GpioUtil::GpioUtil(int display_backlight_pin, int amplifier_power_pin) {
    _chip = gpiod_chip_open_by_number(0);
    _line_amplifier = gpiod_chip_get_line(_chip, amplifier_power_pin);
    _line_display   = gpiod_chip_get_line(_chip, display_backlight_pin);

    gpiod_line_request_output(_line_amplifier, "kitchensound", GPIOD_LINE_ACTIVE_STATE_HIGH);
    gpiod_line_request_output(_line_display, "kitchensound", GPIOD_LINE_ACTIVE_STATE_HIGH);

    turn_off_display();
    turn_off_amplifier();
}

GpioUtil::~GpioUtil() {
    gpiod_line_close_chip(_line_display);
    gpiod_line_close_chip(_line_amplifier);
    gpiod_chip_close(_chip);
}

void GpioUtil::turn_off_amplifier() {
    gpiod_line_set_value(_line_amplifier, 1);
}

void GpioUtil::turn_off_display() {
    gpiod_line_set_value(_line_display, 0);
}

void GpioUtil::turn_on_amplifier() {
    gpiod_line_set_value(_line_amplifier, 0);
}

void GpioUtil::turn_on_display() {
    gpiod_line_set_value(_line_display, 1);
}