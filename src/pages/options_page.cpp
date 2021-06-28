#include "kitchensound/pages/options_page.h"

#include <sstream>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/enum_helper.h"
#include "kitchensound/timeouts.h"
#include "kitchensound/input_event.h"
#include "kitchensound/renderer.h"
#include "kitchensound/os_util.h"
#include "kitchensound/timer.h"
#include "kitchensound/application_backbone.h"
#include "kitchensound/analytics_logger.h"
#include "kitchensound/song_faver.h"

#define PAGE_SIZE 7

#define ENUM_BACKEND_STATE(DO,ACCESSOR) \
    DO(DISABLED,ACCESSOR)               \
    DO(NOT_AVAILABLE,ACCESSOR)          \
    DO(AVAILABLE,ACCESSOR)              \


enum class BACKEND_STATE {
    ENUM_BACKEND_STATE(MAKE_ENUM,)
};

MAKE_ENUM_STRINGIFY(ENUM_BACKEND_STATE, BACKEND_STATE);

struct OptionsPage::Impl {

    Impl(std::unique_ptr<FdRegistry>& fdreg, std::shared_ptr<OsUtil>& os, std::shared_ptr<SongFaver>& faver, std::unique_ptr<AnalyticsLogger>& _analytics)
    : _os{os}, _faver{faver}, _analytics{_analytics}, _model{0, 0, 0, BACKEND_STATE::NOT_AVAILABLE, BACKEND_STATE::NOT_AVAILABLE, {}},
      _value_update{std::make_unique<Timer>(fdreg, "OptionsPage Value Refresh", OS_VALUES_REFRESH, true, [this](){
          refresh_values();
          this->update_model();
      })} {
        refresh_values();
        _model.data.emplace_back("IPv4:\t" + _os->get_local_ip_address());
        _model.data.emplace_back("CPU Temp.:\t" + _os->get_cpu_temperature());
        _model.data.emplace_back("Sys.up.:\t" + _os->get_system_uptime());
        _model.data.emplace_back("Prg.up.:\t" + _os->get_program_uptime());
        _model.data.emplace_back("Analytics:\t Checking");
        _model.data.emplace_back("Song Like:\t Checking");
        _model.data.emplace_back("Shutdown");
        _model.data.emplace_back("Reboot");
        _model.selection_limit = _model.data.size();
    }

    ~Impl() = default;

    void enter_page() {
        refresh_values();
        update_model();
        _model.selection_idx = 0;
        _value_update->reset_timer();
    }

    void leave_page() const {
        _value_update->stop();
    }

    void handle_enter_action() {
        if(_model.selection_idx == 6)
            _os->trigger_shutdown();
        else if(_model.selection_idx == 7)
            _os->trigger_reboot();
    }

    void handle_wheel_input(int delta) {
        _model.selection_idx += delta;
        if(_model.selection_idx < 0)
            _model.selection_idx = 0;
        if(_model.selection_idx >= _model.selection_limit)
            _model.selection_idx = _model.selection_limit -1;

        //is selection still visible? if not make it
        while(_model.selection_offset + PAGE_SIZE <= _model.selection_idx)
            ++_model.selection_offset;
        while(_model.selection_offset > _model.selection_idx)
            --_model.selection_offset;
    }

    void refresh_values() {
        _os->refresh_values();
        _faver->check_backend_availability([this](auto avail) {
            this->_model.song_fav_state = this->determine_new_state(this->_faver->is_enabled(), avail);
        });
        _analytics->check_backend_availability([this](auto avail) {
            this->_model.analytics_state = this->determine_new_state(this->_analytics->is_enabled(), avail);
        });
    }

    static BACKEND_STATE determine_new_state(bool enabled, bool available) {
        if(!enabled)
            return BACKEND_STATE::DISABLED;
        if(available)
            return BACKEND_STATE::AVAILABLE;
        else
            return BACKEND_STATE::NOT_AVAILABLE;
    }

    void update_model() {
        _model.data[0] = "Ipv4:\t" + _os->get_local_ip_address();
        _model.data[1] = "CPU Temp.:\t" + _os->get_cpu_temperature();
        _model.data[2] = "Sys.up.:\t" + _os->get_system_uptime();
        _model.data[3] = "Prg.up.:\t" + _os->get_program_uptime();
        _model.data[4] = "Analytics:\t" + to_string(_model.analytics_state);
        _model.data[5] = "Song Like:\t" + to_string(_model.song_fav_state);
    }

    void render(std::unique_ptr<Renderer>& rend) {
        for (int i = _model.selection_offset; i < _model.selection_limit && i < (_model.selection_offset + PAGE_SIZE); ++i) {
            auto pos = i - _model.selection_offset;
            if(i == _model.selection_idx){
                rend->render_rect(0, 32 + pos*30, 320, 30, Renderer::COLOR::HIGHLIGHT);
            }

            rend->render_text(10, 46 + pos*30, _model.data[i], Renderer::TEXT_SIZE::LARGE, Renderer::TEXT_ALIGN::LEFT);
        }
    }

    static std::string to_string(BACKEND_STATE s) {
        switch (s) {
            case BACKEND_STATE::DISABLED: return "Disabled";
            case BACKEND_STATE::AVAILABLE: return "Available";
            case BACKEND_STATE::NOT_AVAILABLE: return "Not Avail.";
            default: throw std::runtime_error{"Unknown Backend State!"};
        }
    }

    std::shared_ptr<OsUtil> _os;
    std::shared_ptr<SongFaver> _faver;
    std::unique_ptr<AnalyticsLogger>& _analytics;

    std::unique_ptr<Timer> _value_update;

    struct {
        int selection_idx;
        int selection_offset;
        int selection_limit;

        BACKEND_STATE song_fav_state;
        BACKEND_STATE analytics_state;

        std::vector<std::string> data;
    } _model;

};

OptionsPage::OptionsPage(ApplicationBackbone& bb, std::shared_ptr<OsUtil>& os, std::shared_ptr<SongFaver>& faver)
        : BasePage(PAGES::OPTIONS, bb), _impl{std::make_unique<Impl>(bb.fdreg, os, faver, bb.analytics)}
{}

OptionsPage::~OptionsPage() {
    SPDLOG_DEBUG("Dropped Options Page");
}

void OptionsPage::enter_page(PAGES origin, void *payload) {
    _impl->enter_page();
    BasePage::enter_page(origin, payload);
    SPDLOG_INFO("Entered from -> {0}", origin);
}

void *OptionsPage::leave_page(PAGES destination) {
    _impl->leave_page();
    BasePage::leave_page(destination);
    SPDLOG_INFO("Leaving to -> {0}", destination);
    return nullptr;
}

void OptionsPage::handle_enter_key(InputEvent& inev) {
    if(inev.value == INEV_KEY_SHORT){
        _impl->handle_enter_action();
    }
}

void OptionsPage::handle_wheel_input(int delta) {
    _impl->handle_wheel_input(delta);
}

void OptionsPage::render() {
    this->render_time();
    _impl->render(_bb.rend);
}