#ifndef KITCHENSOUND_VOLUME_PAGE_H
#define KITCHENSOUND_VOLUME_PAGE_H

#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/base_page.h"

class StateController;
class Renderer;
class Volume;

class VolumePage : public BasePage {
public:
    VolumePage(PAGES , StateController& , std::shared_ptr<Volume>&);
    ~VolumePage() override;
    void enter_page(PAGES origin, void* payload) override;
    void handle_wheel_input(int delta) override;

protected:
    void render_volume(Renderer& renderer);
private:
    std::shared_ptr<Volume> _volume;
    struct VolumeModel {
        int active_change_timeout;
    } _vol_model;
};

#endif //KITCHENSOUND_VOLUME_PAGE_H
