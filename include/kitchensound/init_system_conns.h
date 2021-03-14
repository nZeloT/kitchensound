#ifndef KITCHENSOUND_SYSTEM_CONN_INIT_H
#define KITCHENSOUND_SYSTEM_CONN_INIT_H

#include <memory>

class Configuration;
class Volume;
class GpioUtil;
class FilePlayback;
class TimeBasedStandby;
class BTController;
class MPDController;
class OsUtil;
class TimerManager;


std::shared_ptr<Volume> init_volume(Configuration&);
std::shared_ptr<GpioUtil> init_gpio(Configuration&);
std::shared_ptr<FilePlayback> init_playback(Configuration&);
std::shared_ptr<TimeBasedStandby> init_standby(Configuration&, TimerManager&);
std::shared_ptr<BTController> init_bt_controller(std::shared_ptr<FilePlayback>&);
std::shared_ptr<MPDController> init_mpd_controller(Configuration&, TimerManager&);
std::shared_ptr<OsUtil> init_os_util(TimerManager&);



#endif //KITCHENSOUND_SYSTEM_CONN_INIT_H
