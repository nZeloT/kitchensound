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
    void set_source_text(std::string const&);
    void set_metadata_text(std::string const&);
    void set_image(std::string const&, std::string const&);

private:
    void update_active_fav_img();
    void update_current_song_fav_state();
    void change_fav_state_of_current_song();

    struct {
        std::string text_source;
        std::string text_metadata;

        void* fav_img_ptr;
        void* unfaved_img_ptr;
        void* syncing_img_ptr;
        void* active_fav_img;
        SongState current_song_state;
        uint64_t current_msg_id;


        std::string current_artwork_ident;
        void* artwork_img_ptr;
    } _model;

    std::shared_ptr<SongFaver> _faver;
    std::unique_ptr<RenderText> _text_source;
    std::unique_ptr<RenderText> _text_metadata;
};

#endif //KITCHENSOUND_PLAYING_PAGE_H
