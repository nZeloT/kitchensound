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

#define DAY_DIVISOR (24 * 60 * 60)
#define HOUR_DIVISOR (60 * 60)
#define MINUTE_DIVISOR (60)

#define DBUS_LOGIN_DESTINATION "org.freedesktop.login1"
#define DBUS_LOGIN_OBJPATH     "/org/freedesktop/login1"
#define DBUS_LOGIN_INTERFACE   "org.freedesktop.login1.Manager"

OsUtil::OsUtil(std::time_t program_start_time) : _last_ip_update{0}, _last_system_uptime_update{0},
                                                 _last_program_uptime_update{0},
                                                 _current_ip{}, _current_system_time{}, _current_program_uptime{},
                                                 _program_start_time{program_start_time} {}

OsUtil::~OsUtil() = default;

std::string OsUtil::get_local_ip_address() {
    if (seconds_since(_last_ip_update) > (24 * 60 * 60))
        update_ip_address();
    return _current_ip;
}

std::string OsUtil::get_program_uptime() {
    if(seconds_since(_last_program_uptime_update) > 30)
        update_program_uptime();
    return _current_program_uptime;
}

std::string OsUtil::get_system_uptime() {
    if (seconds_since(_last_system_uptime_update) > 30)
        update_system_uptime();
    return _current_system_time;
}

void OsUtil::trigger_shutdown() {
    spdlog::info("OsUtil::trigger_shutdown(): Triggering system shutdown");
    try {
        auto proxy = sdbus::createProxy(DBUS_LOGIN_DESTINATION, DBUS_LOGIN_OBJPATH);
        auto method = proxy->createMethodCall(DBUS_LOGIN_INTERFACE, "PowerOff");
        method << false;
        auto reply = proxy->callMethod(method);
    } catch (const sdbus::Error &e) {
        spdlog::error("OsUtil::trigger_shutdown(): failed with {0}", e.getMessage());
    }
}

void OsUtil::trigger_reboot() {
    spdlog::info("OsUtil::trigger_reboot(): triggering system reboot");
    try {
        auto proxy = sdbus::createProxy(DBUS_LOGIN_DESTINATION, DBUS_LOGIN_OBJPATH);
        auto method = proxy->createMethodCall(DBUS_LOGIN_INTERFACE, "Reboot");
        method << false;
        auto reply = proxy->callMethod(method);
    } catch (const sdbus::Error &e) {
        spdlog::error("OsUtil::trigger_reboot(): failed with {0}", e.getMessage());
    }
}

int OsUtil::seconds_since(std::time_t base) {
    return static_cast<int>(std::difftime(std::time(nullptr), base));
}

std::string OsUtil::to_time_string(int secs) {
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

void OsUtil::update_ip_address() {
    spdlog::info("OsUtil::update_ip_address(): Updateing IP Address");
    //code to determine local IP addr is taken from StackOverflow https://stackoverflow.com/a/59025254
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in loopback{};

    if (sock == -1) {
        _current_ip = "Could not socket!";
        return;
    }

    std::memset(&loopback, 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = 1337;   // can be any IP address
    loopback.sin_port = htons(9);      // using debug port

    if (connect(sock, reinterpret_cast<sockaddr *>(&loopback), sizeof(loopback)) == -1) {
        close(sock);
        _current_ip = "Could not connect";
        return;
    }

    socklen_t addrlen = sizeof(loopback);
    if (getsockname(sock, reinterpret_cast<sockaddr *>(&loopback), &addrlen) == -1) {
        close(sock);
        _current_ip = "Could not getsockname";
        return;
    }

    close(sock);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == nullptr) {
        _current_ip = "Could not inet_ntop";
    } else {
        spdlog::info("OptionsPage::get_local_ip(): determined local ip address: {0}", buf);
        _current_ip = std::string{buf};
    }

    spdlog::info("OsUtil::update_ip_address(): Current IP address is {0}", _current_ip);

    _last_ip_update = std::time(nullptr);
}

void OsUtil::update_program_uptime() {
    _current_program_uptime = to_time_string(seconds_since(_program_start_time));
    spdlog::info("OsUtil::update_program_uptime(): Updating program uptime to {0}", _current_program_uptime);
}

void OsUtil::update_system_uptime() {
    std::string uptime = "< ERROR >";
    double uptime_seconds;
    if (std::ifstream("/proc/uptime", std::ios::in) >> uptime_seconds) {
        int secs = static_cast<int>(uptime_seconds);

        uptime = to_time_string(secs);
    }
    _current_system_time = uptime;
    spdlog::info("OsUtil::update_system_uptime(): Updating system update to {0}", _current_system_time);
}