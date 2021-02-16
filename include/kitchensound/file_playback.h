#ifndef KITCHENSOUND_FILE_PLAYBACK_H
#define KITCHENSOUND_FILE_PLAYBACK_H

#include <string>
#include <filesystem>

struct mpg123_handle_struct;
typedef mpg123_handle_struct mpg123_handle;
struct ao_option;
typedef ao_option ao_option;

class FilePlayback {
public:

    FilePlayback(std::string const&, std::string const&);

    ~FilePlayback();

    void playback(std::string const &file);

private:

    mpg123_handle *_mpg_handle;
    unsigned char *_buffer;
    size_t _buffer_size;
    int _alsa_driver_id;
    ao_option *_use_volumedev;
    std::filesystem::path _absolute_res_root;
};

#endif //KITCHENSOUND_FILE_PLAYBACK_H
