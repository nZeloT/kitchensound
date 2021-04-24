#include "kitchensound/init_system_conns.h"

#include <chrono>

#include "kitchensound/config.h"
#include "kitchensound/bt_controller.h"
#include "kitchensound/file_playback.h"
#include "kitchensound/gpio_util.h"
#include "kitchensound/mpd_controller.h"
#include "kitchensound/os_util.h"
#include "kitchensound/song_faver.h"
#include "kitchensound/time_based_standby.h"
#include "kitchensound/volume.h"

std::shared_ptr<Volume> init_volume(std::unique_ptr<Configuration> &conf) {
    return std::make_shared<Volume>(conf->get_default_volume(),
                                    conf->get_alsa_device_name(Configuration::ALSA_DEVICES::MIXER_CONTROL),
                                    conf->get_alsa_device_name(Configuration::ALSA_DEVICES::MIXER_CARD));
}

std::shared_ptr<GpioUtil> init_gpio(std::unique_ptr<Configuration> &conf) {
    return std::make_shared<GpioUtil>(conf->get_gpio_pin(Configuration::GPIO_PIN::DISPLAY_BACKLIGHT),
                                      conf->get_gpio_pin(Configuration::GPIO_PIN::AMPLIFIER_POWER));
}

std::shared_ptr<FilePlayback> init_playback(std::unique_ptr<Configuration> &conf) {
    return std::make_shared<FilePlayback>(conf->get_alsa_device_name(Configuration::ALSA_DEVICES::PCM_DEVICE),
                                          conf->get_res_folder());
}

std::shared_ptr<TimeBasedStandby> init_standby(std::unique_ptr<Configuration> &conf, std::unique_ptr<FdRegistry> &fdr) {
    return std::make_shared<TimeBasedStandby>(conf->get_display_standby(), fdr);
}

std::shared_ptr<BTController>
init_bt_controller(std::unique_ptr<FdRegistry> &fdreg, std::unique_ptr<AnalyticsLogger> &analytics,
                   std::shared_ptr<FilePlayback> &playback) {
    return std::make_shared<BTController>(fdreg, analytics, playback);
}

std::shared_ptr<MPDController>
init_mpd_controller(std::unique_ptr<FdRegistry> &fdr, std::unique_ptr<AnalyticsLogger> &analytics,
                    std::unique_ptr<Configuration> &conf) {
    return std::make_shared<MPDController>(fdr, analytics, conf->get_mpd_config());
}

std::shared_ptr<OsUtil> init_os_util() {
    return std::make_shared<OsUtil>(std::chrono::system_clock::now());
}

std::shared_ptr<SongFaver> init_song_faver(std::unique_ptr<NetworkController>& net, std::unique_ptr<Configuration>& conf) {
    return std::make_shared<SongFaver>(net, conf->get_songfaver_config());
}