#ifndef KITCHENSOUND_VOLUME_CTRL_H
#define KITCHENSOUND_VOLUME_CTRL_H

#include <string>

class Volume {
public:
    Volume(int start_vol, std::string  mixer_control, std::string  mixer_card);
    ~Volume() = default;

    void update_from_system();
    [[nodiscard]] long get_volume() const { return _currentVol; };
    void apply_delta(long delta);
    void set_volume(long target_vol);

private:
    long _currentVol;
    std::string _mixer_control;
    std::string _mixer_card;
};

#endif //KITCHENSOUND_VOLUME_CTRL_H
