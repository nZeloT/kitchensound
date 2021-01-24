#ifndef KITCHENSOUND_SELECTION_PAGE_H
#define KITCHENSOUND_SELECTION_PAGE_H

#include <functional>

#include "kitchensound/render_page.h"
#include "kitchensound/model.h"

template<class T>
class SelectionPage : public BasePage {
public:
    void handle_wheel_input(int delta) override;

    void render(Renderer &renderer) override;

protected:
    SelectionPage(PAGES page, StateController *ctrl,
                  ResourceManager &res, std::vector<T> data,
                  std::function<void *(ResourceManager &res, const T& elem)> get_img,
                  std::function<std::string(const T& elem)> get_text);

    ResourceManager &_res;
    SelectionPageModel<T> _sp_model;
    std::function<void *(ResourceManager &res, const T& elem)> _get_img;
    std::function<std::string(const T& elem)> _get_text;
};

#endif //KITCHENSOUND_SELECTION_PAGE_H
