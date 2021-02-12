#ifndef KITCHENSOUND_RENDERER_H
#define KITCHENSOUND_RENDERER_H

#include <memory>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;
struct SDL_Color;
struct _TTF_Font;
typedef _TTF_Font TTF_Font;

class ResourceManager;

class Renderer {
public:
    explicit Renderer(ResourceManager& res);
    ~Renderer();

    void start_pass();

    void complete_pass();
    enum TEXT_ALIGN {
        LEFT,
        CENTER
    };

    enum TEXT_SIZE {
        SMALL,
        LARGE,
        HUGHE
    };

    enum COLOR {
        BACKGROUND,
        HIGHLIGHT,
        FOREGROUND
    };

    void render_text(int x, int y, const std::string& text, TEXT_SIZE size = LARGE, TEXT_ALIGN alignment = CENTER);

    void render_rect(int x, int y, int w, int h, COLOR color = BACKGROUND);
    void render_image(void* surface, int tlX, int tlY, int w, int h) const;
    void render_image(SDL_Surface* surface, int tlX, int tlY, int w, int h) const;
    void render_texture(SDL_Texture* texture, int tlX, int tlY, int w, int h) const;
private:
    friend class RenderText;

    void load_resources(ResourceManager& res);

    SDL_Texture* create_texture_from_text(std::string const& text, TEXT_SIZE size = LARGE);

    [[nodiscard]] TTF_Font* get_font(TEXT_SIZE size) const;
    void set_active_color(COLOR col);

    TTF_Font *_font_small = nullptr;
    TTF_Font *_font_large = nullptr;
    TTF_Font *_font_hughe = nullptr;

    SDL_Window* _window = nullptr;
    SDL_Renderer *_renderer = nullptr;
};

#endif //KITCHENSOUND_RENDERER_H
