#ifndef KITCHENSOUND_SONG_H
#define KITCHENSOUND_SONG_H

#include <string>
#include <iostream>

#include "kitchensound/enum_helper.h"

#define ENUM_SONG_SOURCE_KIND(DO,ACCESSOR)  \
    DO(RADIO,ACCESSOR)                      \
    DO(BLUETOOTH,ACCESSOR)                  \
    DO(SNAPCAST,ACCESSOR)                   \

#define ENUM_SONG_STATE(DO,ACCESSOR)        \
    DO(DISABLED,ACCESSOR)                   \
    DO(ERROR,ACCESSOR)                      \
    DO(LOADING,ACCESSOR)                    \
    DO(FAVED,ACCESSOR)                      \
    DO(NOT_FAVED,ACCESSOR)                  \


enum class SongSourceKind {
    ENUM_SONG_SOURCE_KIND(MAKE_ENUM,)
};

struct SongSource {
    SongSource(SongSourceKind kind, const std::string &name) : kind(kind), name{name} {}

    SongSourceKind kind;
    std::string name;
};

struct Song {
    explicit Song(const std::string &rawMeta) : raw_meta{rawMeta}, title{}, artist{}, album{} {}

    Song(const std::string &rawMeta, const std::string &title, const std::string &artist)
            : raw_meta(rawMeta), title(title), artist(artist), album{} {}

    Song(const std::string &rawMeta, const std::string &title, const std::string &artist, const std::string &album)
            : raw_meta(rawMeta), title(title), artist(artist), album(album) {}

    std::string raw_meta;
    std::string title;
    std::string artist;
    std::string album;
};

enum class SongState {
    ENUM_SONG_STATE(MAKE_ENUM,)
};

std::ostream& operator<<(std::ostream&,SongSourceKind);
std::ostream& operator<<(std::ostream&,SongState);

#endif //KITCHENSOUND_SONG_H
