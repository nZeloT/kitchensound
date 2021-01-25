#include "kitchensound/pages/selection_page.h"

#define ARROW_RIGHT "img/arrow_right.png"
#define ARROW_LEFT  "img/arrow_left.png"

#define SELECTABLE_COUNT 4

#include "kitchensound/renderer.h"
#include "kitchensound/resource_manager.h"

#include "kitchensound/radio_station_stream.h"
#include "kitchensound/menu_model.h"


template<class T>
SelectionPage<T>::SelectionPage(PAGES page, std::shared_ptr<StateController>& ctrl, std::shared_ptr<ResourceManager>& res, std::vector<T> data,
                                std::function<void *(std::shared_ptr<ResourceManager>&, const T&)> get_img,
                                std::function<std::string(const T&)> get_text)
    : BasePage(page, ctrl), _res(res), _sp_model{}, _get_img{std::move(get_img)}, _get_text{std::move(get_text)} {
    _sp_model.data = std::move(data);
    _sp_model.limit = _sp_model.data.size();
}

template<class T>
SelectionPage<T>::~SelectionPage<T>() = default;

template<class T>
void SelectionPage<T>::handle_wheel_input(int delta) {
    _sp_model.selected += delta;
    if (_sp_model.selected < 0) _sp_model.selected += _sp_model.data.size();
    _sp_model.selected %= _sp_model.limit;
}

template<class T>
void SelectionPage<T>::render(std::unique_ptr<Renderer>& renderer) {
    this->render_time(renderer);
    //each page contains up to four stations
    //render each of the stations in a loop and then place the page indicator below

    for (int i = _sp_model.offset; i < _sp_model.offset + SELECTABLE_COUNT && i < _sp_model.limit; i++) {
        auto element = _sp_model.data[i];

        //0. calculate render offset for the four corners
        auto offsetTile = i % SELECTABLE_COUNT;
        int offsetX = offsetTile % 2 == 0 ? 80 : 240, offsetY = offsetTile < 2 ? 62 : 152;

        //1. render a possible selection background
        if (_sp_model.selected == i) {
            renderer->render_highlight(offsetX-80, offsetY-32, 160, 90);
        }

        //2. render the artwork
        SDL_Rect dstrect = {offsetX - 24, offsetY - 24, 48, 48};
        void* image_ptr = _get_img(_res, element);
        auto image = reinterpret_cast<SDL_Surface*>(image_ptr);
        renderer->render_image(image, dstrect);

        //3. render the element name
        auto text = _get_text(element);
        renderer->render_text_small(offsetX, offsetY+35, text);
    }

    // render the page indicators
    auto has_paging = false;
    if (_sp_model.offset > 0) {
        //render left indicator
        SDL_Rect dstrect{4, 210, 24, 24};
        auto image = reinterpret_cast<SDL_Surface*>(_res->get_static(ARROW_RIGHT));
        renderer->render_image(image, dstrect);
        has_paging = true;
    }
    if (_sp_model.offset + _sp_model.limit < _sp_model.limit) {
        //render right indicator
        SDL_Rect dstrect{292, 210, 24, 24};
        auto image = reinterpret_cast<SDL_Surface*>(_res->get_static(ARROW_LEFT));
        renderer->render_image(image, dstrect);
        has_paging = true;
    }

    //render the page number
    if (has_paging) {
        int page_num = _sp_model.offset / _sp_model.limit;
        renderer->render_text_small(160, 225, std::to_string(page_num));
    }
}

template class SelectionPage<RadioStationStream>;
template class SelectionPage<MenuModel>;