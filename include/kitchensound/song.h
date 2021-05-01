#ifndef KITCHENSOUND_SONG_H
#define KITCHENSOUND_SONG_H

#include <string>
#include <iostream>
#include <sstream>

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

    bool operator!=(Song const& b) const {
        return this->raw_meta != b.raw_meta || this->title != b.title || this->artist != b.artist || this->album != b.album;
    }

    std::string to_string() const {
        if(this->title.empty() || this->artist.empty()) {
            return this->raw_meta;
        }

        std::ostringstream os;
        os << this->title << " - " << this->artist;
        if(!this->album.empty()){
            os << " (" << this->album << ")";
        }

        return os.str();
    }

    std::string raw_meta;
    std::string title;
    std::string artist;
    std::string album;
};

static Song EMPTY_SONG {""};

enum class SongState {
    ENUM_SONG_STATE(MAKE_ENUM,)
};

std::ostream& operator<<(std::ostream&,SongSourceKind);
std::ostream& operator<<(std::ostream&,SongState);

#endif //KITCHENSOUND_SONG_H
