#ifndef KITCHENSOUND_PLAYING_PAGE_H
#define KITCHENSOUND_PLAYING_PAGE_H

#include "kitchensound/song.h"
#include "kitchensound/pages/volume_page.h"

struct ApplicationBackbone;
class RenderText;
class SongFaver;
struct InputEvent;

class PlayingPage : public VolumePage {
public:
    PlayingPage(PAGES page, ApplicationBackbone&, std::shared_ptr<Volume>&, std::shared_ptr<SongFaver>&);
    ~PlayingPage() override;

    void handle_enter_key(InputEvent&) override;
    void enter_page(PAGES origin, void* payload) override;
    void* leave_page(PAGES destination) override;
    void render() override;

protected:
    void set_source(std::string const &v);
    void set_current_song(Song const &song);
    void set_image(std::string const&, std::string const&);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif //KITCHENSOUND_PLAYING_PAGE_H
