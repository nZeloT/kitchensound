#ifndef KITCHENSOUND_OS_UTIL_H
#define KITCHENSOUND_OS_UTIL_H

#include <ctime>
#include <string>

class OsUtil {
public:
    explicit OsUtil(std::time_t);
    ~OsUtil();

    std::string get_local_ip_address();

    std::string get_system_uptime();

    std::string get_program_uptime();

    static void trigger_shutdown();

    static void trigger_reboot();

private:
    void update_ip_address();
    void update_system_uptime();
    void update_program_uptime();
    static int seconds_since(std::time_t);
    static std::string to_time_string(int seconds);

    std::time_t _program_start_time;
    std::string _current_ip;
    std::string _current_system_time;
    std::string _current_program_uptime;

    std::time_t _last_ip_update;
    std::time_t _last_system_uptime_update;
    std::time_t _last_program_uptime_update;

};


#endif //KITCHENSOUND_OS_UTIL_H
