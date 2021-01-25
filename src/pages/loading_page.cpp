#include "kitchensound/pages/loading_page.h"

#include <utility>

#include "kitchensound/renderer.h"

LoadingPage::LoadingPage(std::shared_ptr<StateController> &ctrl) : BasePage(LOADING, ctrl), _model{} {}

LoadingPage::~LoadingPage() = default;

void LoadingPage::render(std::unique_ptr<Renderer> &renderer) {
    this->render_time(renderer);
    renderer->render_text_large(160, 120, _model.msg);
}

void LoadingPage::enter_page(PAGES origin, void* payload) {
    this->update_time();
    _model.msg = "Loading...";
}

void LoadingPage::set_text(const std::string &text) {
    _model.msg = std::string{text};
}