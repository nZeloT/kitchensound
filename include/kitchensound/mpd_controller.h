#ifndef KITCHENSOUND_MPD_CONTROLLER_H
#define KITCHENSOUND_MPD_CONTROLLER_H

#include <string>
#include <functional>
#include <memory>

class mpd_connection;
class SDL_Thread;

class MPDController {
public:
    explicit MPDController(std::function<void(const std::string&)> update_handler);
    ~MPDController();

    void playback_stream(const std::string& stream_url);
    void stop_playback();

    std::function<void(const std::string&)> _update_handler;
private:
    bool _check_and_try();
    void _start_polling();

    void _stop_polling();

    mpd_connection *_connection;
    SDL_Thread *_polling_thread;
};

#endif //KITCHENSOUND_MPD_CONTROLLER_H
