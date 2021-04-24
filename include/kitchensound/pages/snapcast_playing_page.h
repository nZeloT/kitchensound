#ifndef KITCHENSOUND_SNAPCAST_PLAYING_PAGE_H
#define KITCHENSOUND_SNAPCAST_PLAYING_PAGE_H

#include <memory>

#include "kitchensound/pages/playing_page.h"

struct ApplicationBackbone;

class Volume;

class SnapcastController;

class SnapcastPlayingPage : public PlayingPage {
public:
    SnapcastPlayingPage(ApplicationBackbone &, std::shared_ptr<Volume> &, std::shared_ptr<SongFaver> &,
                        std::unique_ptr<SnapcastController> &);

    ~SnapcastPlayingPage();

    void enter_page(PAGES, void *) override;

    void *leave_page(PAGES) override;

    std::unique_ptr<SnapcastController> &_snap;
};


#endif //KITCHENSOUND_SNAPCAST_PLAYING_PAGE_H
