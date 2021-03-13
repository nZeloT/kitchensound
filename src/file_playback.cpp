#include "kitchensound/file_playback.h"

#include <filesystem>

#include <spdlog/spdlog.h>

#include <mpg123.h>
#include <ao/ao.h>

//adapted from http://hzqtc.github.io/2012/05/play-mp3-with-libmpg123-and-libao.html

FilePlayback::FilePlayback(const std::string &pcm_device, const std::string &res_path)
        : _use_volumedev{nullptr}, _alsa_driver_id{-1}, _buffer{nullptr}, _buffer_size{0},
          _mpg_handle{nullptr} {
    _absolute_res_root = std::filesystem::canonical(res_path);
    ao_initialize();
    _alsa_driver_id = ao_driver_id("alsa");
    _use_volumedev = reinterpret_cast<ao_option *>(malloc(sizeof(ao_option)));
    _use_volumedev->key = (char *) "dev";
    _use_volumedev->value = strdup(pcm_device.c_str());
    _use_volumedev->next = nullptr;
    mpg123_init();
    int err;
    _mpg_handle = mpg123_new(nullptr, &err);
    _buffer_size = mpg123_outblock(_mpg_handle);
    _buffer = reinterpret_cast<unsigned char *>(malloc(_buffer_size * sizeof(unsigned char *)));
}

FilePlayback::~FilePlayback() {
    ao_shutdown();
    free(_use_volumedev);
    free(_buffer);
    mpg123_close(_mpg_handle);
    mpg123_delete(_mpg_handle);
    mpg123_exit();
}

void FilePlayback::playback(std::string const &file_name) {
    auto path = std::filesystem::path{_absolute_res_root}.concat("/sound/").concat(file_name);
    SPDLOG_INFO("Trying to playback -> '{0}'", path.string());
    if (mpg123_open(_mpg_handle, path.c_str()) != MPG123_OK) {
        SPDLOG_ERROR("File open error -> {0}", mpg123_strerror(_mpg_handle));
        return;
    };
    int channels, encoding;
    long rate;
    if (mpg123_getformat(_mpg_handle, &rate, &channels, &encoding) != MPG123_OK) {
        SPDLOG_ERROR("Playback error -> {0}", mpg123_strerror(_mpg_handle));
        return;
    };

    ao_sample_format format;
    format.bits = mpg123_encsize(encoding) * 8 /* BITS */;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = nullptr;
    auto dev = ao_open_live(_alsa_driver_id, &format, _use_volumedev);
    if (dev == nullptr) {
        SPDLOG_ERROR("Failed to open ao_device -> {0}", _use_volumedev->value);
        return;
    }

    size_t done;
    while (mpg123_read(_mpg_handle, _buffer, _buffer_size, &done) == MPG123_OK)
        ao_play(dev, reinterpret_cast<char *>(_buffer), done);

    ao_close(dev);
}