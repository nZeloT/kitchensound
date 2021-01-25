#ifndef KITCHENSOUND_VOLUME_PAGE_H
#define KITCHENSOUND_VOLUME_PAGE_H

#include "kitchensound/pages/pages.h"
#include "kitchensound/pages/base_page.h"

class StateController;
class Renderer;
class Volume;

class VolumePage : public BasePage {
public:
    VolumePage(PAGES page, std::shared_ptr<StateController>& ctrl, std::shared_ptr<Volume>& vol);
    ~VolumePage() override;
    void enter_page(PAGES origin, void* payload) override;
    void handle_wheel_input(int delta) override;

protected:
    void render_volume(std::unique_ptr<Renderer>& renderer);
private:
    std::shared_ptr<Volume> _volume;
    struct VolumeModel {
        int active_change_timeout;
    } _vol_model;
};

#endif //KITCHENSOUND_VOLUME_PAGE_H
