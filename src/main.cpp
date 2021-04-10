#include <string>
#include <csignal>

#include <spdlog/spdlog.h>

#include "kitchensound/running.h"
#include "kitchensound/version.h"
#include "kitchensound/sdl_util.h"
#include "kitchensound/application_backbone.h"
#include "kitchensound/input.h"
#include "kitchensound/config.h"
#include "kitchensound/state_controller.h"
#include "kitchensound/renderer.h"
#include "kitchensound/fd_registry.h"
#include "kitchensound/pages/page_loader.h"

void shutdownHandler(int sigint) {
    SPDLOG_INFO("Received software signal -> {0}", std::to_string(sigint));
    running = false;
}

int main(int argc, char **argv) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s - %! : %#] %v");
    log_version_text();

    // set the shutdown handler for SIGINT and SIGTERM
    signal(SIGINT, ::shutdownHandler);
    signal(SIGTERM, ::shutdownHandler);

    // init sdl
    init_sdl();

    ApplicationBackbone b{std::filesystem::path{"../config.conf"}};

    // initialize the render pages
    b.ctrl->register_pages(load_pages(b));
    b.ctrl->setup_inactive_page();

    //4. initialize the input devices
    auto& ctrl = b.ctrl;
    InputSource wheelAxis{b.fdreg, b.conf->get_input_device(Configuration::WHEEL_AXIS), [&ctrl](auto &ev) {
        ctrl->react_wheel_input(ev);
    }};
    InputSource enterKey{b.fdreg, b.conf->get_input_device(Configuration::ENTER_KEY), [&ctrl](auto &ev) {
        ctrl->react_confirm(ev);
    }};
    InputSource menuKey{b.fdreg, b.conf->get_input_device(Configuration::MENU_KEY), [&ctrl](auto &ev) {
        ctrl->react_menu_change(ev);
    }};
    InputSource powerKey{b.fdreg, b.conf->get_input_device(Configuration::POWER_KEY), [&ctrl](auto &ev) {
        ctrl->react_power_change(ev);
    }};

    bool needs_immediate_update = false;
    while (running) {
        if(!needs_immediate_update)
            b.fdreg->wait();

        needs_immediate_update = b.ctrl->update(); // process page transitions

        b.rend->start_pass();
        b.ctrl->render();
        b.rend->complete_pass();
    }

    exit_sdl();

    return 0;
}