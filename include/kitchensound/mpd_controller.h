#ifndef KITCHENSOUND_MPD_CONTROLLER_H
#define KITCHENSOUND_MPD_CONTROLLER_H

#include <string>
#include <functional>
#include <memory>

class mpd_connection;
class SDL_Thread;

class MPDController {
public:
    MPDController(MPDController const&) = delete;
    void operator=(MPDController const&) = delete;

    static MPDController& get() {
        if(!_get()._initialized)
            throw std::runtime_error{"Tried to access uninitialized MPDController!"};
        return _get();
    }
    static void init(std::function<void(const std::string&)> update_handler);

    void playback_stream(const std::string& stream_url);
    void stop_playback();
    std::function<void(const std::string&)> _update_handler;

    ~MPDController();

private:
    static MPDController& _get() {
        static MPDController instance;
        return instance;
    }
    MPDController();

    bool _check_and_try();
    void _start_polling();

    void _stop_polling();

    bool _initialized;
    mpd_connection *_connection;
    SDL_Thread *_polling_thread;
};

#endif //KITCHENSOUND_MPD_CONTROLLER_H
