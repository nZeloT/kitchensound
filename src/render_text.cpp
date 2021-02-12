#include "kitchensound/render_text.h"

#include <SDL.h>

#include "kitchensound/renderer.h"

#define SCROLL_SPEED 1

RenderText::RenderText()
    : _render_width{0}, _render_height{0}, _moves_left{true}, _leftmost_cnt{-1},
      _render_texture{nullptr},  _center_position{0,0}, _current_text{}
{}

RenderText::~RenderText() {
    if(_render_texture != nullptr)
        SDL_DestroyTexture(_render_texture);
}

void RenderText::update_and_render(Renderer& renderer) {
    if(_render_texture != nullptr) {

        if(_render_width > 310) {
            if(_leftmost_cnt == -1)
                _center_position.x += _moves_left ? SCROLL_SPEED : -SCROLL_SPEED;

            if (_center_position.x > 10 && _leftmost_cnt == -1) {
                _moves_left = false;
                _leftmost_cnt = 30;
            }else if (_center_position.x < 310 - _render_width && _leftmost_cnt == -1)
                _moves_left = true;

            if(_leftmost_cnt > -1)
                --_leftmost_cnt;
        }

        renderer.render_texture(_render_texture, _center_position.x, _center_position.y, _render_width, _render_height);
    }
}

void RenderText::change_text(Renderer& renderer, const std::string &new_text, int x, int y) {
    if(_render_texture != nullptr)
        SDL_DestroyTexture(_render_texture);

    _moves_left = true;
    _leftmost_cnt = -1;
    _current_text = std::string{new_text};

    _render_texture = renderer.create_texture_from_text(new_text);
    SDL_QueryTexture(_render_texture, nullptr, nullptr, &_render_width, &_render_height);

    if(_render_width < 310)
        _center_position.x = x - _render_width / 2;
    else
        _center_position.x = 10;

    _center_position.y = y - _render_height / 2;

}