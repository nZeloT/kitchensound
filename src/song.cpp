#include "kitchensound/song.h"

MAKE_ENUM_STRINGIFY(ENUM_SONG_SOURCE_KIND,SongSourceKind)
MAKE_ENUM_STRINGIFY(ENUM_SONG_STATE, SongState)

std::ostream& operator<<(std::ostream& os, Song const& s) {
    os << "Song{'" << s.raw_meta << "'; '" << s.title << "'; '" << s.artist << "'; '" << s.album << "'}";
    return os;
}