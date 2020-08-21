#ifndef KITCHENSOUND_VOLUME_CTRL_H
#define KITCHENSOUND_VOLUME_CTRL_H

class Volume {
public:
    Volume();
    ~Volume() = default;

    void update_from_system();
    [[nodiscard]] long get_volume() const { return _currentVol; };
    void apply_delta(long delta);
    void set_volume(long target_vol);

private:
    long _currentVol;
};

#endif //KITCHENSOUND_VOLUME_CTRL_H
