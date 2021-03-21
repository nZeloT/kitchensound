#ifndef KITCHENSOUND_SELECTION_PAGE_H
#define KITCHENSOUND_SELECTION_PAGE_H

#include <functional>

#include "kitchensound/pages/base_page.h"

struct ApplicationBackbone;

template<class T>
class SelectionPage : public BasePage {
public:
    void handle_wheel_input(int) override;

    void render() override;

protected:
    SelectionPage(PAGES, ApplicationBackbone&,
                  std::vector<T>);
    ~SelectionPage() override;

    void load_images();

    virtual std::string get_text(const T&) = 0;
    virtual void get_image(const T&, void**) = 0;

    struct SelectionPageModel {
        int offset{};
        int selected{};
        int limit{};

        std::vector<T> data;
        std::vector<void*> img_data;
    } _sp_model;
};

#endif //KITCHENSOUND_SELECTION_PAGE_H
