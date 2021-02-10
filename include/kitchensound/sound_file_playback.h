#ifndef KITCHENSOUND_SOUND_FILE_PLAYBACK_H
#define KITCHENSOUND_SOUND_FILE_PLAYBACK_H

#include <string>
#include <filesystem>

void init_playback(const std::string &pcm_device, const std::filesystem::path& path);
void playback_file(std::string const&);
void exit_playback();

#endif //KITCHENSOUND_SOUND_FILE_PLAYBACK_H
