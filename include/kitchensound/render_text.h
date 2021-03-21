#ifndef KITCHENSOUND_RENDER_TEXT_H
#define KITCHENSOUND_RENDER_TEXT_H

#include <string>
#include <memory>

class Renderer;

class FdRegistry;

class Timer;

struct SDL_Texture;

class RenderText {

public:
    RenderText(std::unique_ptr<FdRegistry>&, std::unique_ptr<Renderer>&);
    ~RenderText();

    void set_visible(bool);
    void render();

    void change_text(std::string const& new_text, int x, int y);
    std::string get_current_text() { return _current_text; };

private:
    void update();

    std::unique_ptr<Renderer>& _renderer;
    std::unique_ptr<Timer> _update;

    SDL_Texture* _render_texture;
    struct Point {
        int x;
        int y;
    } _center_position;
    bool         _is_visible;
    int          _render_height;
    int          _render_width;
    bool         _moves_left;
    int          _leftmost_cnt;
    std::string  _current_text;

};

#endif //KITCHENSOUND_RENDER_TEXT_H
