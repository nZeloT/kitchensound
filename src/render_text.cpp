#include "kitchensound/render_text.h"

#include <SDL2/SDL.h>

#include "kitchensound/renderer.h"
#include "kitchensound/fd_registry.h"
#include "kitchensound/timer.h"

#define SCROLL_SPEED 1

RenderText::RenderText(std::unique_ptr<FdRegistry> &fdreg, std::unique_ptr<Renderer> &rend)
        : _render_width{0}, _render_height{0}, _moves_left{true}, _leftmost_cnt{-1},
          _render_texture{nullptr}, _center_position{0, 0}, _current_text{}, _is_visible{false},
          _renderer{rend}, _update{std::make_unique<Timer>(fdreg, "Render Text Movement", 20, true, [this]() {
            this->update();
        })} {}

RenderText::~RenderText() {
    if (_render_texture != nullptr)
        SDL_DestroyTexture(_render_texture);
}

void RenderText::update() {
    if (_render_texture != nullptr) {
        if (_render_width > 310) {
            if (_leftmost_cnt == -1)
                _center_position.x += _moves_left ? SCROLL_SPEED : -SCROLL_SPEED;

            if (_center_position.x > 10 && _leftmost_cnt == -1) {
                _moves_left = false;
                _leftmost_cnt = 30;
            } else if (_center_position.x < 310 - _render_width && _leftmost_cnt == -1)
                _moves_left = true;

            if (_leftmost_cnt > -1)
                --_leftmost_cnt;
        }
    }
}

void RenderText::render() {
    if (_render_texture != nullptr) {
        _renderer->render_texture(_render_texture, _center_position.x, _center_position.y, _render_width,
                                  _render_height);
    }
}

void RenderText::change_text(const std::string &new_text, int x, int y) {
    if (_render_texture != nullptr)
        SDL_DestroyTexture(_render_texture);

    _moves_left = true;
    _leftmost_cnt = -1;
    _current_text = std::string{new_text};

    _render_texture = _renderer->create_texture_from_text(new_text);
    SDL_QueryTexture(_render_texture, nullptr, nullptr, &_render_width, &_render_height);

    if (_render_width < 310)
        _center_position.x = x - _render_width / 2;
    else
        _center_position.x = 10;

    _center_position.y = y - _render_height / 2;

}

void RenderText::set_visible(bool visible) {
    if(visible != _is_visible){
        _is_visible = visible;
        if(_is_visible) {
            _update->reset_timer();
        }else{
            _update->stop();
        }
    }
}