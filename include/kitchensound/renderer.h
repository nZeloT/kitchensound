#ifndef KITCHENSOUND_RENDERER_H
#define KITCHENSOUND_RENDERER_H

#include <memory>
#include <iostream>

#include "kitchensound/enum_helper.h"

#define ENUM_TEXT_ALIGN(DO,ACCESSOR) \
    DO(LEFT,ACCESSOR)                \
    DO(CENTER,ACCESSOR)              \

#define ENUM_TEXT_SIZE(DO,ACCESSOR) \
    DO(SMALL,ACCESSOR)              \
    DO(LARGE,ACCESSOR)              \
    DO(HUGHE,ACCESSOR)              \

#define ENUM_COLOR(DO,ACCESSOR) \
    DO(BACKGROUND,ACCESSOR)     \
    DO(HIGHLIGHT,ACCESSOR)      \
    DO(FOREGROUND,ACCESSOR)     \

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
    explicit Renderer(std::unique_ptr<ResourceManager>& res);
    ~Renderer();

    void start_pass();

    void complete_pass();
    enum class TEXT_ALIGN {
        ENUM_TEXT_ALIGN(MAKE_ENUM,)
    };

    enum class TEXT_SIZE {
        ENUM_TEXT_SIZE(MAKE_ENUM,)
    };

    enum class COLOR {
        ENUM_COLOR(MAKE_ENUM,)
    };

    void render_text(int x, int y, const std::string& text, TEXT_SIZE size = TEXT_SIZE::LARGE, TEXT_ALIGN alignment = TEXT_ALIGN::CENTER);

    void render_rect(int x, int y, int w, int h, COLOR color = COLOR::BACKGROUND);
    void render_image(void* surface, int tlX, int tlY, int w, int h) const;
    void render_image(SDL_Surface* surface, int tlX, int tlY, int w, int h) const;
    void render_texture(SDL_Texture* texture, int tlX, int tlY, int w, int h) const;
private:
    friend class RenderText;

    void load_resources(std::unique_ptr<ResourceManager>& res);

    SDL_Texture* create_texture_from_text(std::string const& text, TEXT_SIZE size = TEXT_SIZE::LARGE);

    [[nodiscard]] TTF_Font* get_font(TEXT_SIZE size) const;
    void set_active_color(COLOR col);

    TTF_Font *_font_small = nullptr;
    TTF_Font *_font_large = nullptr;
    TTF_Font *_font_hughe = nullptr;

    SDL_Window* _window = nullptr;
    SDL_Renderer *_renderer = nullptr;
};

std::ostream& operator<<(std::ostream&,Renderer::TEXT_ALIGN);
std::ostream& operator<<(std::ostream&,Renderer::TEXT_SIZE);
std::ostream& operator<<(std::ostream&,Renderer::COLOR);

#endif //KITCHENSOUND_RENDERER_H
