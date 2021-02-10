#include "kitchensound/sound_file_playback.h"

#include <filesystem>

#include <spdlog/spdlog.h>

#include <mpg123.h>
#include <ao/ao.h>

//adapted from http://hzqtc.github.io/2012/05/play-mp3-with-libmpg123-and-libao.html

struct {
    mpg123_handle *mpg_handle{};
    unsigned char* buffer{};
    size_t buffer_size{};
    int alsa_driver_id{};
    ao_option  use_volumedev{};
    std::filesystem::path absolute_res_root{};
} playback;

void playback_file(std::string const& file_name) {
    spdlog::info("playback_file(): trying to playback `{0}`", file_name);

    auto path = std::filesystem::path{playback.absolute_res_root}.concat("sound/").concat(file_name);
    if(mpg123_open(playback.mpg_handle, path.c_str()) != MPG123_OK){
        spdlog::error("playback_file(): {0}", mpg123_strerror(playback.mpg_handle));
        return;
    };
    int channels, encoding;
    long rate;
    if(mpg123_getformat(playback.mpg_handle, &rate, &channels, &encoding) != MPG123_OK){
        spdlog::error("playback_file(): {0}", mpg123_strerror(playback.mpg_handle));
        return;
    };

    ao_sample_format format;
    format.bits = mpg123_encsize(encoding) * 8 /* BITS */;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = nullptr;
    auto dev = ao_open_live(playback.alsa_driver_id, &format, &playback.use_volumedev);
    if(dev == nullptr){
        spdlog::error("playback_file(): failed to open ao_device!");
        return;
    }

    size_t done;
    while(mpg123_read(playback.mpg_handle, playback.buffer, playback.buffer_size, &done) == MPG123_OK)
        ao_play(dev, reinterpret_cast<char *>(playback.buffer), done);

    ao_close(dev);
}

void init_playback(const std::string& pcm_device, const std::filesystem::path& res_root) {
    playback.absolute_res_root = std::filesystem::absolute(res_root);
    ao_initialize();
    playback.alsa_driver_id = ao_driver_id("alsa");
    playback.use_volumedev.key = (char*)"dev";
    playback.use_volumedev.value = strdup(pcm_device.c_str());
    mpg123_init();
    int err;
    playback.mpg_handle = mpg123_new(nullptr, &err);
    playback.buffer_size = mpg123_outblock(playback.mpg_handle);
    playback.buffer = reinterpret_cast<unsigned char*>(malloc(playback.buffer_size * sizeof(unsigned char*)));
}

void exit_playback() {
    ao_shutdown();
    free(playback.buffer);
    mpg123_close(playback.mpg_handle);
    mpg123_delete(playback.mpg_handle);
    mpg123_exit();
}