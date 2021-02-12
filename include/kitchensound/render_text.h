#ifndef KITCHENSOUND_RENDER_TEXT_H
#define KITCHENSOUND_RENDER_TEXT_H

#include <string>
#include <memory>

class Renderer;

struct SDL_Texture;

class RenderText {

public:
    RenderText();
    ~RenderText();

    void update_and_render(Renderer& renderer);
    void change_text(Renderer& renderer, std::string const& new_text, int x, int y);

    std::string get_current_text() { return _current_text; };

private:

    SDL_Texture* _render_texture;
    struct Point {
        int x;
        int y;
    } _center_position;
    int          _render_height;
    int          _render_width;
    bool         _moves_left;
    int          _leftmost_cnt;
    std::string  _current_text;

};

#endif //KITCHENSOUND_RENDER_TEXT_H
