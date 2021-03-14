#include "kitchensound/pages/loading_page.h"

#include "kitchensound/renderer.h"

LoadingPage::LoadingPage(StateController &ctrl, TimerManager& tm) : BasePage(LOADING, ctrl, tm), _model{} {}

LoadingPage::~LoadingPage() = default;

void LoadingPage::render(Renderer &renderer) {
    this->render_time(renderer);
    renderer.render_text(160, 120, _model.msg, Renderer::LARGE);
}

void LoadingPage::enter_page(PAGES origin, void* payload) {
    _model.msg = "Loading...";
}

void LoadingPage::set_text(const std::string &text) {
    _model.msg = std::string{text};
}