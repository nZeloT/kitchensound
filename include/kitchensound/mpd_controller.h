#ifndef KITCHENSOUND_MPD_CONTROLLER_H
#define KITCHENSOUND_MPD_CONTROLLER_H

#include <string>
#include <functional>
#include <memory>

class mpd_connection;
class SDL_Thread;

class MPDController {
public:
    MPDController();
    ~MPDController();

    void playback_stream(const std::string& stream_url);

    void stop_playback();

    void set_metadata_callback(std::function<void(const std::string&)>);

private:
    bool _check_and_try();

    void _start_polling();
    void _stop_polling();

    mpd_connection *_connection;
    SDL_Thread *_polling_thread;
    std::function<void(const std::string&)> _metadata_callback;
};

#endif //KITCHENSOUND_MPD_CONTROLLER_H
