#include <string>
#include <csignal>
#include <filesystem>

#include <SDL.h>
#include <spdlog/spdlog.h>
#include <wiringPi.h>

#include "kitchensound/running.h"
#include "kitchensound/model.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/renderer.h"
#include "kitchensound/input.h"
#include "kitchensound/config.h"
#include "kitchensound/state_controller.h"

#define DISPLAY_LED_PIN 13

#define VERSION "Version 0.3.1"

void shutdownHandler(int sigint) {
    spdlog::info("Received Software Signal: {0}", std::to_string(sigint));
    running = false;
}

int main(int argc, char **argv) {

    //0. set the shutdown handler for SIGINT and SIGTERM
    signal(SIGINT, ::shutdownHandler);
    signal(SIGTERM, ::shutdownHandler);

    //create a log and a cache directory
    std::filesystem::create_directory("cache");

    //0.1 log the version number
    spdlog::info(VERSION);

    //0.2 init wiringPi
    wiringPiSetupGpio();
    pinMode(4, OUTPUT);
    pinMode(DISPLAY_LED_PIN, OUTPUT);
    digitalWrite(4, 1);
    digitalWrite(DISPLAY_LED_PIN, 1);

    //1. read the configuration file
    Configuration conf{"../config.conf"};

    //2. create Renderer (RAII)
    Renderer renderer{};

    //2.1 create the resource manager (RAII)
    ResourceManager resource{};

    //2.2 initialize the font resources in the renderer
    renderer.load_resources(resource);

    //3. initialize _state controller
    StateController state_ctrl{conf, resource, renderer};

    //4. initialize the input devices (RAII)
    InputSource wheelAxis {conf.get_input_device(Configuration::WHEEL_AXIS), [&state_ctrl](auto& ev) {
        state_ctrl.react_wheel_input(ev.value);
    }};
    InputSource enterKey  {conf.get_input_device(Configuration::ENTER_KEY), [&state_ctrl](auto& ev) {
        if(ev.value == 1) { //key down
            //handling button ENTER input
            state_ctrl.react_confirm();
        }
    }};
    InputSource networkKey{conf.get_input_device(Configuration::NETWORK_KEY), [&state_ctrl](auto& ev) {
        if(ev.value == 1) { //key down
            state_ctrl.react_network_change();
        }
    }};
    InputSource powerKey  {conf.get_input_device(Configuration::LED_KEY), [&state_ctrl](auto& ev) {
        if(ev.value == 1) { //key down
            state_ctrl.react_power_change();
        }
    }};

    bool display_on = true;
    int time_update_counter = 0;
    int time_cntr_reset = 250;
    while (running) {
        //update time regularly
        if(time_update_counter > time_cntr_reset){ //every 5 seconds on avg with 20 ms render delay
            time_update_counter = 0;
        }

        state_ctrl.update(time_update_counter == time_cntr_reset);

        if(!state_ctrl.is_standby_active()) {
            if(!display_on) {
                digitalWrite(DISPLAY_LED_PIN, 1);
                spdlog::info("main(): Tunring Display ON");
                display_on = true;
                time_cntr_reset = 250;
            }
            renderer.start_pass();
            state_ctrl.render();
            renderer.complete_pass();
        }else{
            if(display_on){
                digitalWrite(DISPLAY_LED_PIN, 0);
                spdlog::info("main(): Turning Display OFF");
                display_on = false;
                time_cntr_reset = 30;
            }
        }

        //check if a new input is there to be read
        wheelAxis.check_and_handle();
        enterKey.check_and_handle();
        networkKey.check_and_handle();
        powerKey.check_and_handle();

        SDL_Delay(state_ctrl.is_standby_active() ? 500 : 20);
        ++time_update_counter;
    }

    return 0;
}