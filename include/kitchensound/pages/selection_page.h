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

    void render(std::unique_ptr<Renderer>& renderer) override;

protected:
    SelectionPage(PAGES page, std::shared_ptr<StateController>& ctrl,
                  std::shared_ptr<ResourceManager>& res, std::vector<T> data,
                  std::function<void *(std::shared_ptr<ResourceManager>& res, const T& elem)> get_img,
                  std::function<std::string(const T& elem)> get_text);
    ~SelectionPage() override;
    std::shared_ptr<ResourceManager> _res;

    struct SelectionPageModel {
        int offset;
        int selected;
        int limit;

        std::vector<T> data;
    } _sp_model;
    std::function<void *(std::shared_ptr<ResourceManager>& res, const T& elem)> _get_img;
    std::function<std::string(const T& elem)> _get_text;
};

#endif //KITCHENSOUND_SELECTION_PAGE_H
