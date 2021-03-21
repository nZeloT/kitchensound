#ifndef KITCHENSOUND_PLAYING_PAGE_H
#define KITCHENSOUND_PLAYING_PAGE_H

#include "kitchensound/pages/volume_page.h"

struct ApplicationBackbone;
class RenderText;
struct InputEvent;

class PlayingPage : public VolumePage {
public:
    PlayingPage(PAGES page, ApplicationBackbone&, std::shared_ptr<Volume>&);
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
    struct {
        bool text_source_changed;
        std::string text_source;
        bool text_metadata_changed;
        std::string text_metadata;

        bool fav_update_needed;
        bool faved;
        void* fav_img_ptr;

        std::string current_artwork_ident;
        void* artwork_img_ptr;
    } _model;

    std::unique_ptr<RenderText> _text_source;
    std::unique_ptr<RenderText> _text_metadata;
};

#endif //KITCHENSOUND_PLAYING_PAGE_H
