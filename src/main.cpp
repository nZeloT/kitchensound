#include <string>
#include <csignal>

#include <spdlog/spdlog.h>

#include <chrono>

#include "kitchensound/running.h"
#include "kitchensound/version.h"
#include "kitchensound/sdl_util.h"
#include "kitchensound/config.h"
#include "kitchensound/input.h"
#include "kitchensound/renderer.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/timer_manager.h"
#include "kitchensound/pages/page_loader.h"

void shutdownHandler(int sigint) {
    SPDLOG_INFO("Received software signal -> {0}", std::to_string(sigint));
    running = false;
}

int main(int argc, char **argv) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s - %! : %#] %v");
    log_version_text();

    //0. set the shutdown handler for SIGINT and SIGTERM
    signal(SIGINT, ::shutdownHandler);
    signal(SIGTERM, ::shutdownHandler);

    //0.3 init sdl
    init_sdl();

    auto conf = Configuration{"../config.conf"};

    auto resource_mgr = ResourceManager(conf.get_res_folder(), conf.get_cache_folder());

    auto renderer = Renderer{resource_mgr};

    auto state_ctrl = StateController{};

    auto timer_mgr = TimerManager{};

    //3.1 initialize the render pages
    state_ctrl.register_pages(load_pages(conf, state_ctrl, timer_mgr, resource_mgr));
    state_ctrl.set_active_page(INACTIVE);

    //4. initialize the input devices
    InputSource wheelAxis{timer_mgr, conf.get_input_device(Configuration::WHEEL_AXIS), [&state_ctrl](auto &ev) {
        state_ctrl.react_wheel_input(ev);
    }};
    InputSource enterKey{timer_mgr, conf.get_input_device(Configuration::ENTER_KEY), [&state_ctrl](auto &ev) {
        state_ctrl.react_confirm(ev);
    }};
    InputSource menuKey{timer_mgr, conf.get_input_device(Configuration::MENU_KEY), [&state_ctrl](auto &ev) {
        state_ctrl.react_menu_change(ev);
    }};
    InputSource powerKey{timer_mgr, conf.get_input_device(Configuration::POWER_KEY), [&state_ctrl](auto &ev) {
        state_ctrl.react_power_change(ev);
    }};

    auto new_time = std::chrono::system_clock::now(), last_time = std::chrono::system_clock::now();
    while (running) {
        new_time = std::chrono::system_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(new_time - last_time);

        state_ctrl.update(); // process page transitions
        timer_mgr.update(delta.count());

        renderer.start_pass();
        state_ctrl.render(renderer);
        renderer.complete_pass();

        //check if a new input is there to be read
        wheelAxis.check_and_handle();
        enterKey.check_and_handle();
        menuKey.check_and_handle();
        powerKey.check_and_handle();

        state_ctrl.delay_next_frame();
    }

    exit_sdl();

    return 0;
}