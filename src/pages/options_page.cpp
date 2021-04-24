#include "kitchensound/pages/options_page.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/input_event.h"
#include "kitchensound/renderer.h"
#include "kitchensound/os_util.h"
#include "kitchensound/timer.h"
#include "kitchensound/application_backbone.h"

#define PAGE_SIZE 7

OptionsPage::OptionsPage(ApplicationBackbone& bb, std::shared_ptr<OsUtil>& os)
        : BasePage(PAGES::OPTIONS, bb), _os{os}, _model{0, 0, 0, {}},
        _value_update{std::make_unique<Timer>(bb.fdreg, "OptionsPage Value Refresh", OS_VALUES_REFRESH, true, [this](){
            this->_os->refresh_values();
            this->update_model();
        })}
{
    _os->refresh_values();
    _model.data.emplace_back("IPv4:\t" + _os->get_local_ip_address());
    _model.data.emplace_back("CPU Temp.:\t" + _os->get_cpu_temperature());
    _model.data.emplace_back("Sys.up.:\t" + _os->get_system_uptime());
    _model.data.emplace_back("Prg.up.:\t" + _os->get_program_uptime());
    _model.data.emplace_back("Shutdown");
    _model.data.emplace_back("Reboot");
    _model.selection_limit = _model.data.size();
}

OptionsPage::~OptionsPage() = default;

void OptionsPage::enter_page(PAGES origin, void *payload) {
    _os->refresh_values();
    update_model();
    _model.selection_idx = 0;
    _value_update->reset_timer();
    BasePage::enter_page(origin, payload);
    SPDLOG_INFO("Entered from -> {0}", origin);
}

void *OptionsPage::leave_page(PAGES destination) {
    _value_update->stop();
    BasePage::leave_page(destination);
    SPDLOG_INFO("Leaving to -> {0}", destination);
    return nullptr;
}

void OptionsPage::update_model() {
    _model.data[0] = "Ipv4:\t" + _os->get_local_ip_address();
    _model.data[1] = "CPU Temp.:\t" + _os->get_cpu_temperature();
    _model.data[2] = "Sys.up.:\t" + _os->get_system_uptime();
    _model.data[3] = "Prg.up.:\t" + _os->get_program_uptime();
}

void OptionsPage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT){
        if(_model.selection_idx == 4)
            _os->trigger_shutdown();
        else if(_model.selection_idx == 5)
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

void OptionsPage::render() {
    this->render_time();
    for (int i = _model.selection_offset; i < _model.selection_limit && i < (_model.selection_offset + PAGE_SIZE); ++i) {
        if(i == _model.selection_idx){
            _bb.rend->render_rect(0, 32 + i*30, 320, 30, Renderer::COLOR::HIGHLIGHT);
        }

        _bb.rend->render_text(10, 46 + i*30, _model.data[i], Renderer::TEXT_SIZE::LARGE, Renderer::TEXT_ALIGN::LEFT);
    }
}