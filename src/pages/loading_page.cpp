#include "kitchensound/pages/loading_page.h"

#include "kitchensound/renderer.h"
#include "kitchensound/application_backbone.h"

LoadingPage::LoadingPage(ApplicationBackbone& bb) : BasePage(LOADING, bb), _model{} {}

LoadingPage::~LoadingPage() = default;

void LoadingPage::render() {
    this->render_time();
    _bb.rend->render_text(160, 120, _model.msg, Renderer::LARGE);
}

void LoadingPage::enter_page(PAGES origin, void* payload) {
    BasePage::enter_page(origin, payload);
    _model.msg = "Loading...";
}

void LoadingPage::set_text(const std::string &text) {
    _model.msg = std::string{text};
}