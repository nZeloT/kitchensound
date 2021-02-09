#include "kitchensound/sdl_util.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

#include <spdlog/spdlog.h>

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

void init_sdl2() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        spdlog::error("init_sdl2(): SDL_Init(): {0}", SDL_GetError());
        throw std::runtime_error("Error initializing SDL!");
    }
    spdlog::info("init_sdl2(): SDL_VIDEODRIVER selected : {0}", SDL_GetCurrentVideoDriver());
}

void init_sdl_image() {
    int initflags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(initflags) & initflags) != initflags) {
        spdlog::error("init_sdl_image(): IMG_Init(): {0}", IMG_GetError());
        throw std::runtime_error("Error loading SDL img");
    }
    SDL_version version{};
    SDL_IMAGE_VERSION(&version);
    spdlog::info("init_sdl_image(): Using SDL_IMG_Version: {0}.{1}.{2}",
                 std::to_string(version.major),
                 std::to_string(version.minor),
                 std::to_string(version.patch));
}

void init_sdl_ttf() {
    if (TTF_Init() < 0) {
        spdlog::error("init_sdl_ttf(): {0}", TTF_GetError());
        throw std::runtime_error("Error loading SDL ttf");
    }
    SDL_version version{};
    SDL_TTF_VERSION(&version);
    spdlog::info("init_sdl_ttf: Using SDL_TTF_Version: {0}.{1}.{2}",
                 std::to_string(version.major),
                 std::to_string(version.minor),
                 std::to_string(version.patch));
}