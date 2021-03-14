#include "kitchensound/os_util.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <fstream>

#include <spdlog/spdlog.h>
#include <sdbus-c++/sdbus-c++.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/timer.h"
#include "kitchensound/timer_manager.h"

#define DAY_DIVISOR (24 * 60 * 60)
#define HOUR_DIVISOR (60 * 60)
#define MINUTE_DIVISOR (60)

#define DBUS_LOGIN_DESTINATION "org.freedesktop.login1"
#define DBUS_LOGIN_OBJPATH     "/org/freedesktop/login1"
#define DBUS_LOGIN_INTERFACE   "org.freedesktop.login1.Manager"

struct OsUtil::Impl {
    Impl(TimerManager& tm,  std::time_t program_start_time)
    : _current_ip{}, _current_system_time{}, _current_program_uptime{}, _program_start_time{program_start_time},
      _time_update{tm.request_timer(OS_UTIL_TIME_UPD, true, [this](){
          this->update_program_uptime();
          this->update_system_uptime();
      })},
      _ip_update{tm.request_timer(OS_UTIL_IP_UPD, true, [this](){
          this->update_ip_address();
      })}
      {}

    ~Impl() = default;

    void refresh_values() {
        _time_update.trigger_and_reset();
        _ip_update.trigger_and_reset();
    }

    void update_ip_address() {
        //code to determine local IP addr is taken from StackOverflow https://stackoverflow.com/a/59025254
        int sock = socket(PF_INET, SOCK_DGRAM, 0);
        sockaddr_in loopback{};

        if (sock == -1) {
            _current_ip = "Could not socket!";
            SPDLOG_ERROR("Failed to obtain ip address -> {0}", _current_ip);
            return;
        }

        std::memset(&loopback, 0, sizeof(loopback));
        loopback.sin_family = AF_INET;
        loopback.sin_addr.s_addr = 1337;   // can be any IP address
        loopback.sin_port = htons(9);      // using debug port

        if (connect(sock, reinterpret_cast<sockaddr *>(&loopback), sizeof(loopback)) == -1) {
            close(sock);
            _current_ip = "Could not connect";
            SPDLOG_ERROR("Failed to obtain ip address -> {0}", _current_ip);
            return;
        }

        socklen_t addrlen = sizeof(loopback);
        if (getsockname(sock, reinterpret_cast<sockaddr *>(&loopback), &addrlen) == -1) {
            close(sock);
            _current_ip = "Could not getsockname";
            SPDLOG_ERROR("Failed to obtain ip address -> {0}", _current_ip);
            return;
        }

        close(sock);

        char buf[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == nullptr) {
            _current_ip = "Could not inet_ntop";
            SPDLOG_ERROR("Failed to obtain ip address -> {0}", _current_ip);
        } else {
            _current_ip = std::string{buf};
        }

        SPDLOG_INFO("Determined local ip address -> {0}", _current_ip);
    }

    void update_system_uptime() {
        std::string uptime = "< ERROR >";
        double uptime_seconds;
        if (std::ifstream("/proc/uptime", std::ios::in) >> uptime_seconds) {
            int secs = static_cast<int>(uptime_seconds);

            uptime = to_time_string(secs);
        }
        _current_system_time = uptime;
        SPDLOG_INFO("Updating system uptime -> {0}", _current_system_time);
    }

    void update_program_uptime() {
        _current_program_uptime = to_time_string(seconds_since(_program_start_time));
        SPDLOG_INFO("Updating program uptime -> {0}", _current_program_uptime);
    }


    static int seconds_since(std::time_t base) {
        return static_cast<int>(std::difftime(std::time(nullptr), base));
    }


    static std::string to_time_string(int secs) {
        int days = secs / DAY_DIVISOR;
        secs %= DAY_DIVISOR;

        int hours = secs / HOUR_DIVISOR;
        secs %= HOUR_DIVISOR;

        int minutes = secs / MINUTE_DIVISOR;
        secs %= MINUTE_DIVISOR;

        std::stringstream o;
        o << std::to_string(days) << "d " << std::to_string(hours) << "h " << std::to_string(minutes) << "m "
          << std::to_string(secs) << "s";

        return o.str();
    }

    std::time_t _program_start_time;
    std::string _current_ip;
    std::string _current_system_time;
    std::string _current_program_uptime;


    Timer& _time_update;
    Timer& _ip_update;
};

OsUtil::OsUtil(TimerManager& tm, std::time_t program_start_time)
 : _impl{std::make_unique<Impl>(tm, program_start_time)}
{}

OsUtil::~OsUtil() = default;

void OsUtil::refresh_values() {
    _impl->refresh_values();
}

std::string OsUtil::get_local_ip_address() {
    return _impl->_current_ip;
}

std::string OsUtil::get_program_uptime() {
    return _impl->_current_program_uptime;
}

std::string OsUtil::get_system_uptime() {
    return _impl->_current_system_time;
}

void OsUtil::trigger_shutdown() {
    SPDLOG_INFO("Triggering system shutdown.");
    try {
        auto proxy = sdbus::createProxy(DBUS_LOGIN_DESTINATION, DBUS_LOGIN_OBJPATH);
        auto method = proxy->createMethodCall(DBUS_LOGIN_INTERFACE, "PowerOff");
        method << false;
        auto reply = proxy->callMethod(method);
    } catch (const sdbus::Error &e) {
        SPDLOG_ERROR("Shutdown trigger failed with error -> {0}", e.getMessage());
    }
}

void OsUtil::trigger_reboot() {
    SPDLOG_INFO("Triggering system reboot.");
    try {
        auto proxy = sdbus::createProxy(DBUS_LOGIN_DESTINATION, DBUS_LOGIN_OBJPATH);
        auto method = proxy->createMethodCall(DBUS_LOGIN_INTERFACE, "Reboot");
        method << false;
        auto reply = proxy->callMethod(method);
    } catch (const sdbus::Error &e) {
        SPDLOG_ERROR("Reboot trigger failed with error -> {0}", e.getMessage());
    }
}