#include <string>
#include <csignal>
#include <filesystem>

#include <SDL.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <wiringPi.h>

#include "kitchensound/running.h"
#include "kitchensound/model.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/renderer.h"
#include "kitchensound/input.h"
#include "kitchensound/config.h"
#include "kitchensound/state_controller.h"


void shutdownHandler(int sigint) {
    spdlog::info("Received Software Signal: {0}", std::to_string(sigint));
    running = false;
}

int main(int argc, char **argv) {

    //0. set the shutdown handler for SIGINT
    signal(SIGINT, ::shutdownHandler);

    //create a log and a cache directory
    std::filesystem::create_directory("logs");
    std::filesystem::create_directory("cache");

    //0.1 create the logger
    auto logger = spdlog::daily_logger_st("kitchenlog", "logs/log.txt", 2, 30);
    spdlog::set_default_logger(logger);
    spdlog::flush_on(spdlog::level::info);

    //0.2 init wiringPi
    wiringPiSetupGpio();
    pinMode(4, OUTPUT);
    digitalWrite(4, 1);

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

    int time_update_counter = 0;
    while (running) {
        //update time regularly
        if(time_update_counter > 250){ //every 5 seconds on avg with 20 ms render delay
            time_update_counter = 0;
        }

        state_ctrl.update(time_update_counter == 250);

        renderer.start_pass();
        state_ctrl.render();
        renderer.complete_pass();

        //check if a new input is there to be read
        wheelAxis.check_and_handle();
        enterKey.check_and_handle();
        networkKey.check_and_handle();
        powerKey.check_and_handle();

        SDL_Delay(20);
        ++time_update_counter;
    }

    return 0;
}