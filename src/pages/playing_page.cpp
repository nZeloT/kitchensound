#include "kitchensound/pages/playing_page.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/application_backbone.h"
#include "kitchensound/resource_manager.h"
#include "kitchensound/render_text.h"
#include "kitchensound/renderer.h"
#include "kitchensound/input_event.h"
#include "kitchensound/song_faver.h"

#define FAV_IMAGE   "img/favorite.png"
#define UNFAV_IMAGE "img/favorite_bordered.png"
#define SYNCING_IMG "img/syncing.png"

PlayingPage::PlayingPage(PAGES page, ApplicationBackbone &bb, std::shared_ptr<Volume> &vol,
                         std::shared_ptr<SongFaver> &faver)
        : VolumePage(page, bb, vol), _text_source{std::make_unique<RenderText>(bb.fdreg, bb.rend)},
          _text_metadata{std::make_unique<RenderText>(bb.fdreg, bb.rend)}, _model{}, _faver{faver} {
    _model.current_song_state = _faver->new_msg_id() == -1 ? SongState::DISABLED : SongState::LOADING;
    _model.fav_img_ptr = _bb.res->get_static(FAV_IMAGE);
    _model.unfaved_img_ptr = _bb.res->get_static(UNFAV_IMAGE);
    _model.syncing_img_ptr = _bb.res->get_static(SYNCING_IMG);
    _model.active_fav_img = nullptr;
}

PlayingPage::~PlayingPage() = default;

void PlayingPage::handle_enter_key(InputEvent &inev) {
    if (inev.value == INEV_KEY_LONG
    && (_model.current_song_state == SongState::NOT_FAVED || _model.current_song_state == SongState::FAVED))
        change_fav_state_of_current_song();
}

void PlayingPage::enter_page(PAGES origin, void *payload) {
    VolumePage::enter_page(origin, payload);
    _text_source->set_visible(true);
    _text_metadata->set_visible(true);
}

void *PlayingPage::leave_page(PAGES destination) {
    _text_source->set_visible(false);
    _text_metadata->set_visible(false);
    return VolumePage::leave_page(destination);
}

void PlayingPage::render() {
    this->render_time();
    auto &renderer = _bb.rend;

    //1. render the given artwork
    if (_model.artwork_img_ptr != nullptr)
        renderer->render_image(_model.artwork_img_ptr, 96, 36, 128, 128);

    //2. possibly render the fav icon
    if (_model.active_fav_img != nullptr)
        renderer->render_image(_model.active_fav_img, 278, 36, 32, 32);

    //3. render the station name
    _text_source->render();

    //4. render the meta data
    _text_metadata->render();

    this->render_volume();
}

void PlayingPage::set_source_text(const std::string &v) {
    if (_model.text_source != v) {
        _model.text_source = std::string{v};
        _text_source->change_text(_model.text_source, 160, 180);
        set_metadata_text("");
    }
}

void PlayingPage::set_metadata_text(const std::string &v) {
    if (_model.text_metadata != v) {
        _model.text_metadata = std::string{v};
        _text_metadata->change_text(_model.text_metadata, 160, 210);
        _model.current_song_state = SongState::LOADING;
        _model.current_msg_id = -1;
        update_current_song_fav_state();
    }
}

void PlayingPage::set_image(std::string const &cache_ident, std::string const &static_fallback) {
    _model.current_artwork_ident = cache_ident;
    _bb.res->get_cached(cache_ident, [this, &static_fallback](auto ident, auto data) {
        //check if this callback is still valid and the deemed artwork hasn't changed in the meantime
        if (ident != _model.current_artwork_ident)
            return;

        if (data != nullptr)
            _model.artwork_img_ptr = data;
        else
            _model.artwork_img_ptr = _bb.res->get_static(static_fallback);
    });
}

void PlayingPage::update_active_fav_img() {
    switch (_model.current_song_state) {
        case SongState::DISABLED:
        case SongState::ERROR:
            _model.active_fav_img = nullptr;
            break;
        case SongState::LOADING:
            _model.active_fav_img = _model.syncing_img_ptr;
            break;
        case SongState::NOT_FAVED:
            _model.active_fav_img = _model.unfaved_img_ptr;
            break;
        case SongState::FAVED:
            _model.active_fav_img = _model.fav_img_ptr;
            break;
        default:
            throw std::runtime_error{"Failed to update active image ptr from unknown SongState!"};
    }
}

static SongSourceKind map_page_to_song_source(PAGES p) {
    switch (p) {
        case PAGES::BT_PLAYING: return SongSourceKind::BLUETOOTH;
        case PAGES::SNAPCAST_PLAYING: return SongSourceKind::SNAPCAST;
        case PAGES::STREAM_PLAYING: return SongSourceKind::RADIO;
        default: throw std::runtime_error{"Failed to map PAGES to SongSourceKind!"};
    }
}

void PlayingPage::update_current_song_fav_state() {
    if (_model.current_song_state == SongState::DISABLED)
        return;

    _model.current_song_state = SongState::LOADING;
    _model.current_msg_id = _faver->new_msg_id();

    auto source = SongSource{map_page_to_song_source(_page), _model.text_source};
    auto song = Song{_model.text_metadata}; //TODO adapt backend player to callback with song struct
    _faver->get_state(_model.current_msg_id, source, song, [this](auto msg_id, auto song_state) {
        if(msg_id != this->_model.current_msg_id){
            SPDLOG_INFO("Received song_like message for past msg id => old: {}; current: {}", msg_id, _model.current_msg_id);
            return;
        }

        _model.current_song_state = song_state;
        this->update_active_fav_img();
    });

    update_active_fav_img();
}

void PlayingPage::change_fav_state_of_current_song() {
    if (_model.current_song_state == SongState::DISABLED || _model.current_song_state == SongState::LOADING)
        return;

    _model.current_msg_id = _faver->new_msg_id();

    auto source = SongSource{map_page_to_song_source(_page), _model.text_source};
    auto song = Song{_model.text_metadata};
    auto cb = [this](auto msg_id, auto song_state) {
        if(msg_id != this->_model.current_msg_id){
            SPDLOG_INFO("Received song_like message for past msg id => old: {}; current: {}", msg_id, _model.current_msg_id);
            return;
        }

        _model.current_song_state = song_state;
        this->update_active_fav_img();
    };

    switch(_model.current_song_state){
        case SongState::NOT_FAVED:
            _faver->fav_song(_model.current_msg_id, source, song, std::move(cb));
            break;

        case SongState::FAVED:
            _faver->unfav_song(_model.current_msg_id, source, song, std::move(cb));
            break;

        case SongState::LOADING:
        case SongState::DISABLED:
        case SongState::ERROR:
            throw std::runtime_error{"Tried to Fav or unfav song with loading or disabled state!"};
    }

    _model.current_song_state = SongState::LOADING;
    update_active_fav_img();
}