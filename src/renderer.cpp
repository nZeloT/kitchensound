#include "kitchensound/renderer.h"

#include <stdexcept>
#include <string>

#include <spdlog/spdlog.h>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#define COLOR_BACKGROUND  61, 137,  49
#define COLOR_HIGHLIGHT  104, 211,  82
#define COLOR_FOREGROUND 255, 255, 255

Renderer::Renderer()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        spdlog::error("Renderer::C-Tor(): SDL_Init(): {0}", SDL_GetError());
        throw std::runtime_error("Error initializing SDL!");
    }
    spdlog::info("Renderer::C-Tor(): SDL_VIDEODRIVER selected : {0}", SDL_GetCurrentVideoDriver());

    window = SDL_CreateWindow
            (
                    "Kitchensound",
                    0, 0,
                    320, 240,
                    SDL_WINDOW_FULLSCREEN
            );
    if (nullptr == window) {
        spdlog::error("Renderer::C-Tor(): SDL_CreateWindow(): {0}", SDL_GetError());
        throw std::runtime_error("Error creating SDL Window");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (nullptr == renderer) {
        spdlog::error("Renderer::C-Tor(): SDL_CreateRenderer(): {0}", SDL_GetError());
        throw std::runtime_error("Error creating SDL renderer");
    }
    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    spdlog::info("Renderer::C-Tor(): SDL_RENDER_DRIVER selected : {0}", info.name);

    SDL_ShowCursor(SDL_DISABLE);

    init_sdl_ttf();
    init_sdl_image();
    spdlog::info("Renderer::C-Tor(): Initialization finished!");
}

Renderer::~Renderer() {
    IMG_Quit();
    TTF_CloseFont(font_small);
    TTF_CloseFont(font_large);
    TTF_CloseFont(font_hughe);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Renderer::init_sdl_ttf() {
    if (TTF_Init() < 0) {
        spdlog::error("Renderer::init_sdl_ttf(): {0}", TTF_GetError());
        throw std::runtime_error("Error loading SDL ttf");
    }
    SDL_version version{};
    SDL_TTF_VERSION(&version);
    spdlog::info("Renderer::init_sdl_ttf(): Using SDL_TTF_Version: {0}.{1}.{2}",
                 std::to_string(version.major),
                 std::to_string(version.minor),
                 std::to_string(version.patch));
}

void Renderer::init_sdl_image() {
    int initflags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(initflags) & initflags) != initflags) {
        spdlog::error("Renderer::init_sdl_image(): IMG_Init(): {0}", IMG_GetError());
        throw std::runtime_error("Error loading SDL img");
    }
    SDL_version version{};
    SDL_IMAGE_VERSION(&version);
    spdlog::info("Renderer::init_sdl_image(): Using SDL_IMG_Version: {0}.{1}.{2}",
                 std::to_string(version.major),
                 std::to_string(version.minor),
                 std::to_string(version.patch));
}

void Renderer::render_text_small(int x, int y, const std::string &text) {
    render_text_centered(text,font_small, {x, y}, get_color(FOREGROUND));
}

void Renderer::render_text_large(int x, int y, const std::string &text) {
    render_text_centered(text,font_large, {x, y}, get_color(FOREGROUND));
}

void Renderer::render_text_hughe(int x, int y, const std::string &text) {
    render_text_centered(text,font_hughe, {x, y}, get_color(FOREGROUND));
}

void Renderer::render_foreground(int x, int y, int w, int h) {
    set_color(FOREGROUND);
    render_rectangle(x, y, w, h);
}

void Renderer::render_highlight(int x, int y, int w, int h) {
    set_color(HIGHLIGHT);
    render_rectangle(x, y, w, h);
}

void Renderer::render_background(int x, int y, int w, int h) {
    set_color(BACKGROUND);
    render_rectangle(x, y, w, h);
}

void Renderer::set_color(COLOR_PALETTE color) const {
    switch (color) {
        case BACKGROUND:
            SDL_SetRenderDrawColor(renderer, COLOR_BACKGROUND, SDL_ALPHA_OPAQUE);
            break;
        case FOREGROUND:
            SDL_SetRenderDrawColor(renderer, COLOR_FOREGROUND, SDL_ALPHA_OPAQUE);
            break;
        case HIGHLIGHT:
            SDL_SetRenderDrawColor(renderer, COLOR_HIGHLIGHT, SDL_ALPHA_OPAQUE);
            break;
        default:
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            break;
    }
}

SDL_Color Renderer::get_color(COLOR_PALETTE color) {
    switch (color) {
        case BACKGROUND:
            return {COLOR_BACKGROUND, SDL_ALPHA_OPAQUE};
        case FOREGROUND:
            return {COLOR_FOREGROUND, SDL_ALPHA_OPAQUE};
        case HIGHLIGHT:
            return {COLOR_HIGHLIGHT, SDL_ALPHA_OPAQUE};
        default:
            return {255, 0, 0, SDL_ALPHA_OPAQUE};
    }
}

void Renderer::render_text_centered(const std::string &text, TTF_Font *font, SDL_Point point, SDL_Color fg) const {
    SDL_Surface *surface = TTF_RenderUTF8_Solid(font, text.c_str(), fg);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    int texW = 0, texH = 0;
    SDL_QueryTexture(texture, nullptr, nullptr, &texW, &texH);
    SDL_Rect dstrect{point.x - texW / 2, point.y - texH / 2, texW, texH};
    SDL_RenderCopy(renderer, texture, nullptr, &dstrect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void Renderer::render_image(SDL_Surface *surface, SDL_Rect &destrect) const {
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_RenderCopy(renderer, texture, nullptr, &destrect);
    SDL_DestroyTexture(texture);
}

void Renderer::render_rectangle(int x, int y, int w, int h) const {
    SDL_Rect rect{x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void Renderer::load_resources(ResourceManager& res) {
    font_small = reinterpret_cast<TTF_Font*>(res.get_static("SMALL"));
    font_large = reinterpret_cast<TTF_Font*>(res.get_static("LARGE"));
    font_hughe = reinterpret_cast<TTF_Font*>(res.get_static("HUGHE"));
    spdlog::info("Renderer::load_resources(): Loaded font resources.");
}

void Renderer::start_pass() {
    //0. fill the background
    set_color(BACKGROUND);
    SDL_RenderFillRect(renderer, nullptr);
}

void Renderer::complete_pass() {
    SDL_RenderPresent(renderer);
}
