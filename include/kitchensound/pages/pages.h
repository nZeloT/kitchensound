#ifndef KITCHENSOUND_PAGES_H
#define KITCHENSOUND_PAGES_H

#include <iostream>

#include "kitchensound/enum_helper.h"

#define ENUM_PAGES(DO,ACCESSOR)     \
    DO(INACTIVE,ACCESSOR)           \
    DO(LOADING,ACCESSOR)            \
    DO(STREAM_SELECTION,ACCESSOR)   \
    DO(STREAM_PLAYING,ACCESSOR)     \
    DO(BT_PLAYING,ACCESSOR)         \
    DO(OPTIONS,ACCESSOR)            \
    DO(MENU_SELECTION,ACCESSOR)     \
    DO(SNAPCAST_PLAYING,ACCESSOR)   \


enum class PAGES {
    ENUM_PAGES(MAKE_ENUM,)
};

std::ostream& operator<<(std::ostream&,PAGES);


#endif //KITCHENSOUND_PAGES_H
