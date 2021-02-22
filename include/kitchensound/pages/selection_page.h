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
                  ResourceManager& res,
                  std::vector<T> data);
    ~SelectionPage() override;

    void load_images();

    virtual std::string get_text(const T&) = 0;
    virtual void get_image(const T&, void**) = 0;

    ResourceManager& _res;

    struct SelectionPageModel {
        int offset{};
        int selected{};
        int limit{};

        std::vector<T> data;
        std::vector<void*> img_data;
    } _sp_model;
};

#endif //KITCHENSOUND_SELECTION_PAGE_H
