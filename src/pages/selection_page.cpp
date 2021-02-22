#include "kitchensound/pages/selection_page.h"

#define ARROW_RIGHT "img/arrow_right.png"
#define ARROW_LEFT  "img/arrow_left.png"

#define SELECTABLE_COUNT 4

#include "kitchensound/renderer.h"
#include "kitchensound/resource_manager.h"

#include "kitchensound/radio_station_stream.h"
#include "kitchensound/menu_model.h"


template<class T>
SelectionPage<T>::SelectionPage(PAGES page, StateController& ctrl, ResourceManager& res,
                                std::vector<T> data)
    : BasePage(page, ctrl), _res(res), _sp_model{}{
    _sp_model.data = std::move(data);
    _sp_model.limit = _sp_model.data.size();
    for(auto& e : _sp_model.data)
        _sp_model.img_data.push_back(nullptr);
}

template<class T>
SelectionPage<T>::~SelectionPage<T>() = default;

template<class T>
void SelectionPage<T>::handle_wheel_input(int delta) {
    _sp_model.selected += delta;
    if (_sp_model.selected < 0) _sp_model.selected += _sp_model.data.size();
    _sp_model.selected %= _sp_model.limit;
}

template <class T>
void SelectionPage<T>::load_images() {
    for(int i = 0; i < _sp_model.limit; i++){
        get_image(_sp_model.data[i], &_sp_model.img_data[i]);
    }
}

template<class T>
void SelectionPage<T>::render(Renderer& renderer) {
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
            renderer.render_rect(offsetX-80, offsetY-32, 160, 90, Renderer::HIGHLIGHT);
        }

        //2. render the artwork
        auto image = _sp_model.img_data[i];
        if(image != nullptr)
            renderer.render_image(image, offsetX - 24, offsetY - 24, 48, 48);

        //3. render the element name
        auto text = get_text(element);
        renderer.render_text(offsetX, offsetY+35, text, Renderer::SMALL);
    }

    // render the page indicators
    auto has_paging = false;
    if (_sp_model.offset > 0) {
        //render left indicator
        auto image = reinterpret_cast<SDL_Surface*>(_res.get_static(ARROW_RIGHT));
        renderer.render_image(image, 4, 210, 24, 24);
        has_paging = true;
    }
    if (_sp_model.offset + _sp_model.limit < _sp_model.limit) {
        //render right indicator
        auto image = reinterpret_cast<SDL_Surface*>(_res.get_static(ARROW_LEFT));
        renderer.render_image(image, 292, 210, 24, 24);
        has_paging = true;
    }

    //render the page number
    if (has_paging) {
        int page_num = _sp_model.offset / _sp_model.limit;
        renderer.render_text(160, 225, std::to_string(page_num), Renderer::SMALL);
    }
}

template class SelectionPage<RadioStationStream>;
template class SelectionPage<MenuModel>;