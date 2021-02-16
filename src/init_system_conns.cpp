#include "kitchensound/init_system_conns.h"

#include "kitchensound/config.h"
#include "kitchensound/bt_controller.h"
#include "kitchensound/file_playback.h"
#include "kitchensound/gpio_util.h"
#include "kitchensound/mpd_controller.h"
#include "kitchensound/time_based_standby.h"
#include "kitchensound/volume.h"

std::shared_ptr<Volume> init_volume(Configuration &conf) {
    return std::make_shared<Volume>(conf.get_default_volume(),
                                    conf.get_alsa_device_name(Configuration::MIXER_CONTROL),
                                    conf.get_alsa_device_name(Configuration::MIXER_CARD));
}

std::shared_ptr<GpioUtil> init_gpio(Configuration &conf) {
    return std::make_shared<GpioUtil>(conf.get_gpio_pin(Configuration::DISPLAY_BACKLIGHT),
                                      conf.get_gpio_pin(Configuration::AMPLIFIER_POWER));
}

std::shared_ptr<FilePlayback> init_playback(Configuration &conf) {
    return std::make_shared<FilePlayback>(conf.get_alsa_device_name(Configuration::PCM_DEVICE),
                                          conf.get_res_folder());
}

std::shared_ptr<TimeBasedStandby> init_standby(Configuration &conf) {
    return std::make_shared<TimeBasedStandby>(conf.get_display_standby());
}

std::shared_ptr<BTController> init_bt_controller(std::shared_ptr<FilePlayback>& playback){
    return std::make_shared<BTController>(playback);
}

std::shared_ptr<MPDController> init_mpd_controller(Configuration& conf) {
    return std::make_shared<MPDController>();
}