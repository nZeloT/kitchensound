#include "kitchensound/pages/options_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/renderer.h"
#include <kitchensound/os_util.h>

OptionsPage::OptionsPage(StateController &ctrl, std::shared_ptr<OsUtil>& os)
        : BasePage(OPTIONS, ctrl), _os{os} {}

OptionsPage::~OptionsPage() = default;

void OptionsPage::enter_page(PAGES origin, void *payload) {
    update_time();
    spdlog::info("OptionsPage::enter_page(): from origin {0}", origin);
}

void *OptionsPage::leave_page(PAGES destination) {
    spdlog::info("OptionsPage::leave_page(): to destination {0}", destination);
    return nullptr;
}

void OptionsPage::handle_enter_key() {
    _os->trigger_reboot();
}

void OptionsPage::handle_wheel_input(int delta) {
    //NOP
}

void OptionsPage::render(Renderer& renderer) {
    this->render_time(renderer);
    renderer.render_text(10, 40, "IPv4:\t" + _os->get_local_ip_address(), Renderer::LARGE, Renderer::LEFT);
    renderer.render_text(10, 70,  "Sys. Up.: " + _os->get_system_uptime(), Renderer::LARGE, Renderer::LEFT);
    renderer.render_text(10, 100, "Prg. Up.: " + _os->get_program_uptime(), Renderer::LARGE, Renderer::LEFT);
    renderer.render_text(10, 130, "Shutdown", Renderer::LARGE, Renderer::LEFT);
    renderer.render_text(10, 160, "Reboot", Renderer::LARGE, Renderer::LEFT);
}