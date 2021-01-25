#ifndef KITCHENSOUND_RENDERER_H
#define KITCHENSOUND_RENDERER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <memory>

class ResourceManager;

class Renderer {
public:
    Renderer();
    ~Renderer();

    void load_resources(std::shared_ptr<ResourceManager>& res);

    void start_pass();
    void complete_pass();

    void render_text_small(int x, int y, const std::string& text);
    void render_text_large(int x, int y, const std::string& text);
    void render_text_hughe(int x, int y, const std::string& text);
    void render_foreground(int x, int y, int w, int h);
    void render_highlight(int x, int y, int w, int h);
    void render_background(int x, int y, int w, int h);
    void render_image(SDL_Surface* surface, SDL_Rect& destrect) const;

private:
    friend class RenderText;

    enum COLOR_PALETTE {
        BACKGROUND,
        HIGHLIGHT,
        FOREGROUND
    };

    void set_color(COLOR_PALETTE color) const;
    static SDL_Color get_color(COLOR_PALETTE color);
    void render_text_centered(const std::string &text, TTF_Font *font, SDL_Point point, SDL_Color fg) const;
    void render_rectangle(int x, int y, int w, int h) const;

    void init_sdl_image();
    void init_sdl_ttf();

    TTF_Font *font_small = nullptr;
    TTF_Font *font_large = nullptr;
    TTF_Font *font_hughe = nullptr;

    SDL_Window* window = nullptr;
    SDL_Renderer *renderer = nullptr;

};

#endif //KITCHENSOUND_RENDERER_H
