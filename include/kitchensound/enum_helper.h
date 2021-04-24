#ifndef KITCHENSOUND_ENUM_HELPER_H
#define KITCHENSOUND_ENUM_HELPER_H

#define MAKE_ENUM(VAL,ACCESSOR) VAL,

#define MAKE_ENUM_CASES(VAL,ACCESSOR)                           \
    case ACCESSOR::VAL: os << #VAL; break;                      \


#define MAKE_ENUM_STRINGIFY(CREATOR, ACCESSOR)                  \
    std::ostream& operator<<(std::ostream& os, ACCESSOR val) {  \
        switch(val) {                                           \
            CREATOR(MAKE_ENUM_CASES,ACCESSOR)                   \
            default: os.setstate(std::ios::failbit);            \
        }                                                       \
        return os;                                              \
    };                                                          \

#endif //KITCHENSOUND_ENUM_HELPER_H
