#include "kitchensound/sdl_util.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

void init_sdl2();
void init_sdl_image();
void init_sdl_ttf();
void init_sdl() {
    init_sdl2();
    init_sdl_image();
    init_sdl_ttf();
}

void exit_sdl() {
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void delay(int ms) {
    SDL_Delay(ms);
}

void init_sdl2() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SPDLOG_ERROR("sdl_init error -> {0}", SDL_GetError());
        throw std::runtime_error("Error initializing SDL!");
    }
    SPDLOG_INFO("sdl_videodriver -> {0}", SDL_GetCurrentVideoDriver());
}

void init_sdl_image() {
    int initflags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(initflags) & initflags) != initflags) {
        SPDLOG_ERROR("sdl_img error -> {0}", IMG_GetError());
        throw std::runtime_error("Error loading SDL img");
    }
    SDL_version version{};
    SDL_IMAGE_VERSION(&version);
    SPDLOG_INFO("Using SDL_IMG_Version: {0}.{1}.{2}",
                 std::to_string(version.major),
                 std::to_string(version.minor),
                 std::to_string(version.patch));
}

void init_sdl_ttf() {
    if (TTF_Init() < 0) {
        SPDLOG_ERROR("sdl_ttf error -> {0}", TTF_GetError());
        throw std::runtime_error("Error loading SDL ttf");
    }
    SDL_version version{};
    SDL_TTF_VERSION(&version);
    SPDLOG_INFO("Using SDL_TTF_Version: {0}.{1}.{2}",
                 std::to_string(version.major),
                 std::to_string(version.minor),
                 std::to_string(version.patch));
}