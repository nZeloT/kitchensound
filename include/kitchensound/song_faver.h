#ifndef KITCHENSOUND_SONG_FAV_H
#define KITCHENSOUND_SONG_FAV_H

#include <memory>
#include <functional>

#include "kitchensound/song.h"
#include "kitchensound/config.h"

class NetworkController;

class SongFaver {

public:
    explicit SongFaver(std::unique_ptr<NetworkController> &, Configuration::SongFaverConfig);

    ~SongFaver();

    uint64_t new_msg_id();

    void get_state(uint64_t, SongSource const &, Song const &, std::function<void(uint64_t, SongState)>);

    void fav_song(uint64_t, SongSource const &, Song const &, std::function<void(uint64_t, SongState)>);

    void unfav_song(uint64_t, SongSource const &, Song const &, std::function<void(uint64_t, SongState)>);

    bool is_enabled();

    void check_backend_availability(std::function<void(bool)>);

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;

};

#endif //KITCHENSOUND_SONG_FAV_H
