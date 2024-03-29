#include "kitchensound/gpio_util.h"

#include <gpiod.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

GpioUtil::GpioUtil(int display_backlight_pin, int amplifier_power_pin)
 : _line_display{nullptr}, _line_amplifier{nullptr}, _chip{nullptr} {
    _chip = gpiod_chip_open_by_number(0);
    _line_amplifier = gpiod_chip_get_line(_chip, amplifier_power_pin);
    _line_display   = gpiod_chip_get_line(_chip, display_backlight_pin);

    gpiod_line_request_output(_line_amplifier, "kitchensound", GPIOD_LINE_ACTIVE_STATE_HIGH);
    gpiod_line_request_output(_line_display, "kitchensound", GPIOD_LINE_ACTIVE_STATE_HIGH);

    turn_off_display();
    turn_off_amplifier();
}

GpioUtil::~GpioUtil() {
    SPDLOG_DEBUG("Dropping GpioUtil ...");
    gpiod_line_release(_line_display);
    gpiod_line_release(_line_amplifier);
    gpiod_chip_close(_chip);
    SPDLOG_DEBUG("Dropped GpioUtil");
}

void GpioUtil::turn_off_amplifier() {
    gpiod_line_set_value(_line_amplifier, 1);
    SPDLOG_INFO("Turn amplifier OFF");
}

void GpioUtil::turn_off_display() {
    gpiod_line_set_value(_line_display, 0);
    SPDLOG_INFO("Turn display OFF");
}

void GpioUtil::turn_on_amplifier() {
    gpiod_line_set_value(_line_amplifier, 0);
    SPDLOG_INFO("Turn amplifier ON");
}

void GpioUtil::turn_on_display() {
    gpiod_line_set_value(_line_display, 1);
    SPDLOG_INFO("Turn display ON");
}