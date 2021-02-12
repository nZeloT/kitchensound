#ifndef KITCHENSOUND_SELECTION_PAGE_H
#define KITCHENSOUND_SELECTION_PAGE_H

#include <functional>

#include "kitchensound/pages/base_page.h"

class StateController;
class ResourceManager;
class Renderer;

template<class T>
class SelectionPage : public BasePage {
public:
    void handle_wheel_input(int delta) override;

    void render(Renderer& renderer) override;

protected:
    SelectionPage(PAGES page, StateController& ctrl,
                  ResourceManager& res, std::vector<T> data,
                  std::function<void *(ResourceManager& res, const T& elem)> get_img,
                  std::function<std::string(const T& elem)> get_text);
    ~SelectionPage() override;
    ResourceManager& _res;

    struct SelectionPageModel {
        int offset;
        int selected;
        int limit;

        std::vector<T> data;
    } _sp_model;
    std::function<void *(ResourceManager& res, const T& elem)> _get_img;
    std::function<std::string(const T& elem)> _get_text;
};

#endif //KITCHENSOUND_SELECTION_PAGE_H
