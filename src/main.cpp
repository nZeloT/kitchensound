#include <string>
#include <csignal>

#include <spdlog/spdlog.h>

#include "kitchensound/running.h"
#include "kitchensound/version.h"
#include "kitchensound/sdl_util.h"
#include <kitchensound/gpio_util.h>
#include "kitchensound/config.h"
#include "kitchensound/volume.h"
#include "kitchensound/input.h"
#include "kitchensound/renderer.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/pages/page_loader.h"
#include "kitchensound/sound_file_playback.h"
#include "kitchensound/time_based_standby.h"

void shutdownHandler(int sigint) {
    spdlog::info("Received Software Signal: {0}", std::to_string(sigint));
    running = false;
}

int main(int argc, char **argv) {
    log_version_text();

    //0. set the shutdown handler for SIGINT and SIGTERM
    signal(SIGINT, ::shutdownHandler);
    signal(SIGTERM, ::shutdownHandler);

    //0.3 init sdl
    init_sdl();

    auto conf = Configuration{"../config.conf"};

    auto resource_mgr = ResourceManager(conf.get_res_folder(), conf.get_cache_folder());

    auto renderer = Renderer{resource_mgr};

    auto volume = Volume(conf.get_default_volume(),
                         conf.get_alsa_device_name(Configuration::MIXER_CONTROL),
                         conf.get_alsa_device_name(Configuration::MIXER_CARD));

    auto gpio = GpioUtil{conf.get_gpio_pin(Configuration::DISPLAY_BACKLIGHT),
                         conf.get_gpio_pin(Configuration::AMPLIFIER_POWER)};

    //2.5 init alsa playback mechanics
    init_playback(conf.get_alsa_device_name(Configuration::PCM_DEVICE),
                  conf.get_res_folder());

    auto standby = TimeBasedStandby{conf.get_display_standby()};

    auto state_ctrl = StateController{standby};

    //3.1 initialize the render pages
    state_ctrl.register_pages(load_pages(conf, state_ctrl, resource_mgr, volume, gpio));
    state_ctrl.set_active_page(INACTIVE);

    //4. initialize the input devices (RAII)
    InputSource wheelAxis{conf.get_input_device(Configuration::WHEEL_AXIS), [&state_ctrl](auto &ev) {
        state_ctrl.react_wheel_input(ev.value);
    }};
    InputSource enterKey{conf.get_input_device(Configuration::ENTER_KEY), [&state_ctrl](auto &ev) {
        if (ev.value == 1) { //key down
            //handling button ENTER input
            state_ctrl.react_confirm();
        }
    }};
    InputSource menuKey{conf.get_input_device(Configuration::MENU_KEY), [&state_ctrl](auto &ev) {
        if (ev.value == 1) { //key down
            state_ctrl.react_menu_change();
        }
    }};
    InputSource powerKey{conf.get_input_device(Configuration::POWER_KEY), [&state_ctrl](auto &ev) {
        if (ev.value == 1) { //key down
            state_ctrl.react_power_change();
        }
    }};

    bool display_on = false;
    int time_update_counter = 0;
    int time_cntr_reset = 250;
    while (running) {
        //update time regularly
        if (time_update_counter > time_cntr_reset) { //every 5 seconds on avg with 20 ms render delay
            time_update_counter = 0;
        }

        state_ctrl.update(time_update_counter == time_cntr_reset);

        if (!standby.is_standby_active()) {
            if (!display_on) {
                gpio.turn_on_display(); //TODO move to inactive page
                spdlog::info("main(): Turning Display ON");
                display_on = true;
                time_cntr_reset = 250;
            }
            renderer.start_pass();
            state_ctrl.render(renderer);
            renderer.complete_pass();
        } else {
            if (display_on) {
                gpio.turn_off_display();
                spdlog::info("main(): Turning Display OFF");
                display_on = false;
                time_cntr_reset = 30;
            }
        }

        //check if a new input is there to be read
        wheelAxis.check_and_handle();
        enterKey.check_and_handle();
        menuKey.check_and_handle();
        powerKey.check_and_handle();

        delay(standby.is_standby_active() ? 500 : 20);
        ++time_update_counter;
    }

    exit_playback();
    exit_sdl();

    return 0;
}