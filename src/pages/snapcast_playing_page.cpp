#include "kitchensound/pages/snapcast_playing_page.h"

#include <spdlog/spdlog.h>

#include "kitchensound/pages/pages.h"
#include "kitchensound/song.h"
#include "kitchensound/snapcast_controller.h"

SnapcastPlayingPage::SnapcastPlayingPage(ApplicationBackbone &bb, std::shared_ptr<Volume> &vol,
                                         std::shared_ptr<SongFaver> &faver, std::shared_ptr<SnapcastController> &snap)
        : PlayingPage(PAGES::SNAPCAST_PLAYING, bb, vol, faver), _snap{snap} {
    set_image("", "img/speaker_group.png");
    set_source("Snapcast Multiroom");
    _snap->set_metadata_cb([this](auto& song) {
        this->set_current_song(song);
    });
}

SnapcastPlayingPage::~SnapcastPlayingPage() {
    SPDLOG_DEBUG("Dropped Snapcast Playing Page");
}


void SnapcastPlayingPage::enter_page(PAGES origin, void *payload) {
    PlayingPage::enter_page(origin, payload);
    reset_metadata();
    _snap->start_snapclient_service();
}


void *SnapcastPlayingPage::leave_page(PAGES dest) {
    _snap->stop_snapclient_service();
    return PlayingPage::leave_page(dest);
}

void SnapcastPlayingPage::reset_metadata() {
    set_current_song(EMPTY_SONG);
}