#include "kitchensound/pages/playing_page.h"

#include "kitchensound/application_backbone.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/render_text.h"
#include "kitchensound/renderer.h"
#include "kitchensound/input_event.h"

#define FAV_IMAGE "img/favorite.png"

PlayingPage::PlayingPage(PAGES page, ApplicationBackbone& bb, std::shared_ptr<Volume> &vol)
 : VolumePage(page, bb, vol), _text_source{std::make_unique<RenderText>(bb.fdreg, bb.rend)},
 _text_metadata{std::make_unique<RenderText>(bb.fdreg, bb.rend)}, _model{} {
    _model.fav_img_ptr = _bb.res->get_static(FAV_IMAGE);
}

 PlayingPage::~PlayingPage() = default;

void PlayingPage::handle_enter_key(InputEvent &inev) {
    if(inev.value == INEV_KEY_LONG)
        _model.faved = true;
}

void PlayingPage::enter_page(PAGES origin, void *payload) {
    VolumePage::enter_page(origin, payload);
    _text_source->set_visible(true);
    _text_metadata->set_visible(true);
}

void* PlayingPage::leave_page(PAGES destination) {
    _text_source->set_visible(false);
    _text_metadata->set_visible(false);
    return VolumePage::leave_page(destination);
}

void PlayingPage::render() {
    this->render_time();
    auto& renderer = _bb.rend;

    //1. render the given artwork
    if(_model.artwork_img_ptr != nullptr)
        renderer->render_image(_model.artwork_img_ptr, 96, 36, 128, 128);

    //2. possibly render the fav icon
    if(_model.faved)
        renderer->render_image(_model.fav_img_ptr, 278, 36, 32, 32);

    //3. render the station name
    if (_model.text_source_changed) {
        _model.text_source_changed = false;
        _text_source->change_text(_model.text_source, 160, 180);
    }

    _text_source->render();

    //4. render the meta data
    if (_model.text_metadata_changed) {
        _model.text_metadata_changed = false;
        _text_metadata->change_text(_model.text_metadata, 160, 210);
    }

    _text_metadata->render();

    this->render_volume();
}

void PlayingPage::set_source_text(const std::string &v) {
    if(_model.text_source != v) {
        _model.text_source = std::string{v};
        _model.text_source_changed = true;
        set_metadata_text("");
    }
}

void PlayingPage::set_metadata_text(const std::string &v) {
    if(_model.text_metadata != v) {
        _model.text_metadata = std::string {v};
        _model.text_metadata_changed = true;
        _model.faved = false;
        _model.fav_update_needed = true;
    }
}

void PlayingPage::set_image(std::string const& cache_ident, std::string const& static_fallback) {
    _model.current_artwork_ident = cache_ident;
    _bb.res->get_cached(cache_ident, [this, &static_fallback](auto ident, auto data){
        //check if this callback is still valid and the deemed artwork hasn't changed in the meantime
        if(ident != _model.current_artwork_ident)
            return;

        if(data != nullptr)
            _model.artwork_img_ptr = data;
        else
            _model.artwork_img_ptr = _bb.res->get_static(static_fallback);
    });
}