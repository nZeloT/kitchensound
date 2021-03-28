#include "kitchensound/os_util.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <fstream>

#include <systemd/sd-bus.h>
#include <spdlog/spdlog.h>

#define DAY_DIVISOR (24 * 60 * 60)
#define HOUR_DIVISOR (60 * 60)
#define MINUTE_DIVISOR (60)

#define DBUS_LOGIN_DESTINATION "org.freedesktop.login1"
#define DBUS_LOGIN_OBJPATH     "/org/freedesktop/login1"
#define DBUS_LOGIN_INTERFACE   "org.freedesktop.login1.Manager"

struct OsUtil::Impl {
    explicit Impl(std::chrono::time_point<std::chrono::system_clock> program_start_time)
            : _current_ip{}, _current_system_time{}, _current_program_uptime{},
              _program_start_time{program_start_time} {}

    ~Impl() = default;

    void refresh_values() {
        update_ip_address();
        update_system_uptime();
        update_program_uptime();
        update_cpu_temperature();
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
        auto diff = std::chrono::system_clock::now() - _program_start_time;
        long sec_since_start = std::chrono::duration_cast<std::chrono::seconds>(diff).count();

        _current_program_uptime = to_time_string(sec_since_start);
        SPDLOG_INFO("Updating program uptime -> {0}", _current_program_uptime);
    }

    void update_cpu_temperature() {
        std::string temp = "< ERROR >";
        int t;
        if(std::ifstream{"/sys/class/thermal/thermal_zone0/temp", std::ios::in} >> t) {
            float tc = static_cast<float>(t) / 1000;
            temp = std::to_string(tc) + " °C";
        }
        _current_cpu_temp = temp;
        SPDLOG_INFO("Updating cpu temperature -> {}", _current_cpu_temp);
    }

    static std::string to_time_string(long secs) {
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

    std::chrono::time_point<std::chrono::system_clock> _program_start_time;
    std::string _current_ip;
    std::string _current_system_time;
    std::string _current_program_uptime;
    std::string _current_cpu_temp;
};

OsUtil::OsUtil(std::chrono::time_point<std::chrono::system_clock> program_start_time)
        : _impl{std::make_unique<Impl>(program_start_time)} {}

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

std::string OsUtil::get_cpu_temperature() {
    return _impl->_current_cpu_temp;
}

static void check_error(int r, std::string const& msg) {
    if (r < 0)
        throw std::runtime_error{msg};
}

void OsUtil::trigger_shutdown() {
    SPDLOG_INFO("Triggering system shutdown.");

    sd_bus *bus;
    auto r = sd_bus_open_system(&bus);
    check_error(r, "Failed to open system dbus!");

    r = sd_bus_call_method(bus, DBUS_LOGIN_DESTINATION, DBUS_LOGIN_OBJPATH, DBUS_LOGIN_INTERFACE, "PowerOff", nullptr,
                           nullptr, "b", false);
    check_error(r, "Failed to trigger shutdown!");

    sd_bus_close(bus);
}

void OsUtil::trigger_reboot() {
    SPDLOG_INFO("Triggering system reboot.");

    sd_bus *bus;
    auto r = sd_bus_open_system(&bus);
    check_error(r, "Failed to open system dbus!");

    r = sd_bus_call_method(bus, DBUS_LOGIN_DESTINATION, DBUS_LOGIN_OBJPATH, DBUS_LOGIN_INTERFACE, "Reboot", nullptr,
                           nullptr, "b", false);
    check_error(r, "Failed to trigger reboot!");

    sd_bus_close(bus);
}