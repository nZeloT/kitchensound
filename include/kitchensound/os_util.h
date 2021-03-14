#ifndef KITCHENSOUND_OS_UTIL_H
#define KITCHENSOUND_OS_UTIL_H

#include <ctime>
#include <string>
#include <memory>

class TimerManager;

class OsUtil {
public:
    OsUtil(TimerManager&, std::time_t);
    ~OsUtil();

    std::string get_local_ip_address();

    std::string get_system_uptime();

    std::string get_program_uptime();

    static void trigger_shutdown();

    static void trigger_reboot();

    void refresh_values();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};


#endif //KITCHENSOUND_OS_UTIL_H
