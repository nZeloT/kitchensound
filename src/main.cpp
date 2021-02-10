#include <string>
#include <csignal>

#include <SDL.h>
#include <spdlog/spdlog.h>
#include <wiringPi.h>

#include "kitchensound/running.h"
#include "kitchensound/version.h"
#include "kitchensound/config.h"
#include "kitchensound/renderer.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/volume.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/input.h"
#include "kitchensound/pages/page_loader.h"
#include "kitchensound/sdl_util.h"
#include "kitchensound/sound_file_playback.h"

void shutdownHandler(int sigint) {
    spdlog::info("Received Software Signal: {0}", std::to_string(sigint));
    running = false;
}

int main(int argc, char **argv) {

    //0. set the shutdown handler for SIGINT and SIGTERM
    signal(SIGINT, ::shutdownHandler);
    signal(SIGTERM, ::shutdownHandler);

    //0.1 log the version number
    spdlog::info(get_version_string());

    //0.2 init wiringPi
    wiringPiSetupGpio();

    //0.3 init sdl
    init_sdl();

    //1. read the configuration file
    auto conf = std::make_unique<Configuration>("../config.conf");

    //2. create Renderer (RAII)
    auto renderer = std::make_unique<Renderer>();

    //2.1 create the resource manager (RAII)
    auto resource_mgr = std::make_shared<ResourceManager>(conf->get_res_folder(), conf->get_cache_folder());

    //2.2 initialize the font resources in the renderer
    renderer->load_resources(resource_mgr);

    //2.3 initialize the volume handler
    auto volume = std::make_shared<Volume>(conf->get_default_volume(),
                                           conf->get_alsa_device_name(Configuration::MIXER_CONTROL),
                                           conf->get_alsa_device_name(Configuration::MIXER_CARD));

    //2.4 initialize gpio
    auto amp_power = conf->get_gpio_pin(Configuration::AMPLIFIER_POWER);
    auto display_back = conf->get_gpio_pin(Configuration::DISPLAY_BACKLIGHT);
    pinMode(amp_power, OUTPUT);
    pinMode(display_back, OUTPUT);
    digitalWrite(amp_power, 1);
    digitalWrite(display_back, 1);

    //2.5 init alsa playback mechanics
    init_playback(conf->get_alsa_device_name(Configuration::PCM_DEVICE),
                  conf->get_res_folder());

    //3. initialize state controller
    auto state_ctrl = std::make_shared<StateController>(conf);

    //3.1 initialize the render pages
    state_ctrl->register_pages(load_pages(conf, state_ctrl, resource_mgr, volume));
    state_ctrl->set_active_page(INACTIVE);

    //4. initialize the input devices (RAII)
    InputSource wheelAxis{conf->get_input_device(Configuration::WHEEL_AXIS), [&state_ctrl](auto &ev) {
        state_ctrl->react_wheel_input(ev.value);
    }};
    InputSource enterKey{conf->get_input_device(Configuration::ENTER_KEY), [&state_ctrl](auto &ev) {
        if (ev.value == 1) { //key down
            //handling button ENTER input
            state_ctrl->react_confirm();
        }
    }};
    InputSource menuKey{conf->get_input_device(Configuration::MENU_KEY), [&state_ctrl](auto &ev) {
        if (ev.value == 1) { //key down
            state_ctrl->react_menu_change();
        }
    }};
    InputSource powerKey{conf->get_input_device(Configuration::POWER_KEY), [&state_ctrl](auto &ev) {
        if (ev.value == 1) { //key down
            state_ctrl->react_power_change();
        }
    }};

    bool display_on = true;
    int time_update_counter = 0;
    int time_cntr_reset = 250;
    while (running) {
        //update time regularly
        if (time_update_counter > time_cntr_reset) { //every 5 seconds on avg with 20 ms render delay
            time_update_counter = 0;
        }

        state_ctrl->update(time_update_counter == time_cntr_reset);

        if (!state_ctrl->is_standby_active()) {
            if (!display_on) {
                digitalWrite(display_back, 1); //TODO move to state controller
                spdlog::info("main(): Turning Display ON");
                display_on = true;
                time_cntr_reset = 250;
            }
            renderer->start_pass();
            state_ctrl->render(renderer);
            renderer->complete_pass();
        } else {
            if (display_on) {
                digitalWrite(display_back, 0);
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

        SDL_Delay(state_ctrl->is_standby_active() ? 500 : 20);
        ++time_update_counter;
    }

    exit_playback();
    exit_sdl();

    return 0;
}