#include "kitchensound/volume.h"

#include <alsa/asoundlib.h>
#include <spdlog/spdlog.h>

Volume::Volume(int start_vol, const std::string& mixer_control, const std::string& mixer_card)
: _currentVol{0}, _mixer_control{}, _mixer_card{} {
    _mixer_card = {mixer_card};
    _mixer_control = {mixer_control};
    update_from_system();
    set_volume(start_vol);
}

Volume::~Volume() {
    SPDLOG_DEBUG("Dropped Volume");
}

void Volume::apply_delta(long delta) {
    if (_currentVol + delta < 0
        || _currentVol + delta > 100)
        throw std::runtime_error("Tried to apply invalid delta!");
    set_volume(_currentVol + delta);
}

void Volume::set_volume(long target_vol) {
    if (target_vol < 0
        || target_vol > 100)
        throw std::runtime_error("Tried to set invalid volume out of rankge [0;100]");

    //Taken from https://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
    // Credits go to SO User @trenki
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, _mixer_card.c_str());
    snd_mixer_selem_register(handle, nullptr, nullptr);
    snd_mixer_load(handle);

    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, _mixer_control.c_str());
    snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, target_vol * max / 100);

    snd_mixer_close(handle);

    _currentVol = target_vol;
}

void Volume::update_from_system() {
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, _mixer_card.c_str());
    snd_mixer_selem_register(handle, nullptr, nullptr);
    snd_mixer_load(handle);

    snd_mixer_selem_id_malloc(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, _mixer_control.c_str());
    snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem,
                                        _snd_mixer_selem_channel_id::SND_MIXER_SCHN_FRONT_LEFT,
                                        &_currentVol);

    snd_mixer_close(handle);
}
