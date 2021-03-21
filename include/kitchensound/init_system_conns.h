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
class FdRegistry;


std::shared_ptr<Volume> init_volume(std::unique_ptr<Configuration>&);
std::shared_ptr<GpioUtil> init_gpio(std::unique_ptr<Configuration>&);
std::shared_ptr<FilePlayback> init_playback(std::unique_ptr<Configuration>&);
std::shared_ptr<TimeBasedStandby> init_standby(std::unique_ptr<Configuration>&, std::unique_ptr<FdRegistry>&);
std::shared_ptr<BTController> init_bt_controller(std::shared_ptr<FilePlayback>&, std::unique_ptr<FdRegistry>&);
std::shared_ptr<MPDController> init_mpd_controller(std::unique_ptr<Configuration>&, std::unique_ptr<FdRegistry>&);
std::shared_ptr<OsUtil> init_os_util();



#endif //KITCHENSOUND_SYSTEM_CONN_INIT_H
