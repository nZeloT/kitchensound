#include <string>
#include <csignal>

#include <spdlog/spdlog.h>

#include "kitchensound/running.h"
#include "kitchensound/version.h"
#include "kitchensound/sdl_util.h"
#include "kitchensound/config.h"
#include "kitchensound/input.h"
#include "kitchensound/renderer.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/pages/page_loader.h"

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

    auto state_ctrl = StateController{};

    //3.1 initialize the render pages
    state_ctrl.register_pages(load_pages(conf, state_ctrl, resource_mgr));
    state_ctrl.set_active_page(INACTIVE);

    //4. initialize the input devices
    InputSource wheelAxis{conf.get_input_device(Configuration::WHEEL_AXIS), [&state_ctrl](auto &ev) {
        state_ctrl.react_wheel_input(ev);
    }};
    InputSource enterKey{conf.get_input_device(Configuration::ENTER_KEY), [&state_ctrl](auto &ev) {
        state_ctrl.react_confirm(ev);
    }};
    InputSource menuKey{conf.get_input_device(Configuration::MENU_KEY), [&state_ctrl](auto &ev) {
        state_ctrl.react_menu_change(ev);
    }};
    InputSource powerKey{conf.get_input_device(Configuration::POWER_KEY), [&state_ctrl](auto &ev) {
        state_ctrl.react_power_change(ev);
    }};

    while (running) {
        state_ctrl.update();

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