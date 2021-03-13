#include "kitchensound/pages/options_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/input_event.h"
#include "kitchensound/renderer.h"
#include <kitchensound/os_util.h>

#define PAGE_SIZE 7

OptionsPage::OptionsPage(StateController &ctrl, std::shared_ptr<OsUtil>& os)
        : BasePage(OPTIONS, ctrl), _os{os}, _model{0, 0, 0, {}} {
    _model.data.emplace_back("IPv4:\t" + _os->get_local_ip_address());
    _model.data.emplace_back("Sys.up.:\t" + _os->get_system_uptime());
    _model.data.emplace_back("Prg.up.:\t" + _os->get_program_uptime());
    _model.data.emplace_back("Shutdown");
    _model.data.emplace_back("Reboot");
    _model.selection_limit = _model.data.size();
}

OptionsPage::~OptionsPage() = default;

void OptionsPage::enter_page(PAGES origin, void *payload) {
    update_time();
    _model.selection_idx = 0;
    SPDLOG_INFO("Entered from -> {0}", origin);
}

void *OptionsPage::leave_page(PAGES destination) {
    SPDLOG_INFO("Leaving to -> {0}", destination);
    return nullptr;
}

void OptionsPage::update_time() {
    BasePage::update_time();
    _model.data[0] = "Ipv4:\t" + _os->get_local_ip_address();
    _model.data[1] = "Sys.up.:\t" + _os->get_system_uptime();
    _model.data[2] = "Prg.up.:\t" + _os->get_program_uptime();
}

void OptionsPage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT){
        if(_model.selection_idx == 3)
            _os->trigger_shutdown();
        else if(_model.selection_idx == 4)
            _os->trigger_reboot();
    }
}

void OptionsPage::handle_wheel_input(int delta) {
    _model.selection_idx += delta;
    if(_model.selection_idx < 0)
        _model.selection_idx = 0;
    if(_model.selection_idx >= _model.selection_limit)
        _model.selection_idx = _model.selection_limit -1;
}

void OptionsPage::render(Renderer& renderer) {
    this->render_time(renderer);
    for (int i = _model.selection_offset; i < _model.selection_limit && i < (_model.selection_offset + PAGE_SIZE); ++i) {
        if(i == _model.selection_idx){
            renderer.render_rect(0, 32 + i*30, 320, 30, Renderer::HIGHLIGHT);
        }

        renderer.render_text(10, 46 + i*30, _model.data[i], Renderer::LARGE, Renderer::LEFT);
    }
}