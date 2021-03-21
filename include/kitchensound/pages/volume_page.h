#ifndef KITCHENSOUND_VOLUME_PAGE_H
#define KITCHENSOUND_VOLUME_PAGE_H

#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/base_page.h"

struct ApplicationBackbone;
class Volume;

class VolumePage : public BasePage {
public:
    VolumePage(PAGES , ApplicationBackbone&, std::shared_ptr<Volume>&);
    ~VolumePage() override;
    void handle_wheel_input(int) override;
    void* leave_page(PAGES) override;

protected:
    void render_volume();
private:
    std::shared_ptr<Volume> _volume;
    std::unique_ptr<Timer> _volume_bar_timeout;
    bool _volume_bar_visible;
};

#endif //KITCHENSOUND_VOLUME_PAGE_H
