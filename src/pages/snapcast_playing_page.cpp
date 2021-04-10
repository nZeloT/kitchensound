#include "kitchensound/pages/snapcast_playing_page.h"

#include "kitchensound/pages/pages.h"
#include "kitchensound/snapcast_controller.h"

SnapcastPlayingPage::SnapcastPlayingPage(ApplicationBackbone& bb, std::shared_ptr<Volume>& vol, std::unique_ptr<SnapcastController>& snap)
    : PlayingPage(SNAPCAST_PLAYING, bb, vol), _snap{snap} {
    set_image("", "img/speaker_group.png");
    set_source_text("Snapcast Multiroom");
}

SnapcastPlayingPage::~SnapcastPlayingPage() = default;


void SnapcastPlayingPage::enter_page(PAGES origin, void *payload) {
    PlayingPage::enter_page(origin, payload);
    _snap->start_snapclient_service();
}


void * SnapcastPlayingPage::leave_page(PAGES dest) {
    _snap->stop_snapclient_service();
    return PlayingPage::leave_page(dest);
}