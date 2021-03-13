#include "kitchensound/renderer.h"

#include <stdexcept>
#include <string>

#include <SDL.h>
#include <SDL_ttf.h>

#include <spdlog/spdlog.h>

#include "kitchensound/resource_manager.h"

#define COLOR_BACKGROUND  61, 137,  49
#define COLOR_HIGHLIGHT  104, 211,  82
#define COLOR_FOREGROUND 255, 255, 255

Renderer::Renderer(ResourceManager& res)
{
    _window = SDL_CreateWindow
            (
                    "Kitchensound",
                    0, 0,
                    320, 240,
                    SDL_WINDOW_FULLSCREEN
            );
    if (nullptr == _window) {
        SPDLOG_ERROR("sdl create window error -> {0}", SDL_GetError());
        throw std::runtime_error("Error creating SDL Window");
    }

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    if (nullptr == _renderer) {
        SPDLOG_ERROR("sdl create renderer error -> {0}", SDL_GetError());
        throw std::runtime_error("Error creating SDL _renderer");
    }
    SDL_RendererInfo info;
    SDL_GetRendererInfo(_renderer, &info);
    SPDLOG_INFO("sdl render driver -> {0}", info.name);

    SDL_ShowCursor(SDL_DISABLE);

    load_resources(res);

    SPDLOG_INFO("Renderer initialized");
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
}

void Renderer::load_resources(ResourceManager& res) {
    _font_small = reinterpret_cast<TTF_Font*>(res.get_static("SMALL"));
    _font_large = reinterpret_cast<TTF_Font*>(res.get_static("LARGE"));
    _font_hughe = reinterpret_cast<TTF_Font*>(res.get_static("HUGHE"));
    SPDLOG_INFO("Obtained font resources");
}

void Renderer::start_pass() {
    //0. fill the background
    set_active_color(BACKGROUND);
    SDL_RenderFillRect(_renderer, nullptr);
}

void Renderer::complete_pass() {
    SDL_RenderPresent(_renderer);
}

void Renderer::render_rect(int x, int y, int w, int h, COLOR col) {
    set_active_color(col);
    SDL_Rect rect{x, y, w, h};
    SDL_RenderFillRect(_renderer, &rect);
}

void Renderer::render_text(int x, int y, const std::string &text, TEXT_SIZE size, TEXT_ALIGN alignment) {
    auto texture = create_texture_from_text(text, size);
    int texW = 0, texH = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
    SDL_Rect dstrect{x - texW / 2, y - texH / 2, texW, texH};
    if(alignment == LEFT)
        dstrect.x = x;
    SDL_RenderCopy(_renderer, texture, nullptr, &dstrect);
    SDL_DestroyTexture(texture);
}

void Renderer::render_image(void * surface, int tlX, int tlY, int w, int h) const {
    auto srf = reinterpret_cast<SDL_Surface *>(surface);
    render_image(srf, tlX, tlY, w, h);
}

void Renderer::render_image(SDL_Surface *surface, int tlX, int tlY, int w, int h) const {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(_renderer, surface);
    render_texture(texture, tlX, tlY, w, h);
    SDL_DestroyTexture(texture);
}

void Renderer::render_texture(SDL_Texture *texture, int tlX, int tlY, int w, int h) const {
    SDL_Rect destrect{tlX, tlY, w, h};
    SDL_RenderCopy(_renderer, texture, nullptr, &destrect);
}

SDL_Texture* Renderer::create_texture_from_text(const std::string &text, TEXT_SIZE size) {
    SDL_Color col{COLOR_FOREGROUND, SDL_ALPHA_OPAQUE};
    auto surface = TTF_RenderUTF8_Solid(get_font(size), text.c_str(), col);
    auto texture =  SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

TTF_Font* Renderer::get_font(TEXT_SIZE size) const {
    switch (size) {
        case SMALL:
            return _font_small;
        case LARGE:
            return _font_large;
        case HUGHE:
            return _font_hughe;
        default:
            throw std::runtime_error{"Tried to access unknown font size!"};
    }
}

void Renderer::set_active_color(COLOR color) {
    switch (color) {
        case BACKGROUND:
            SDL_SetRenderDrawColor(_renderer, COLOR_BACKGROUND, SDL_ALPHA_OPAQUE);
            break;
        case FOREGROUND:
            SDL_SetRenderDrawColor(_renderer, COLOR_FOREGROUND, SDL_ALPHA_OPAQUE);
            break;
        case HIGHLIGHT:
            SDL_SetRenderDrawColor(_renderer, COLOR_HIGHLIGHT, SDL_ALPHA_OPAQUE);
            break;
        default:
            SDL_SetRenderDrawColor(_renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            break;
    }
}
