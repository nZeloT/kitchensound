#include "kitchensound/pages/options_page.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iostream>

#include <spdlog/spdlog.h>
#include <sdbus-c++/sdbus-c++.h>

#include "kitchensound/renderer.h"

#define DAY_DIVISOR (24 * 60 * 60)
#define HOUR_DIVISOR (60 * 60)
#define MINUTE_DIVISOR (60)

#define DBUS_DESTINATION "org.freedesktop.login1"
#define DBUS_OBJPATH     "/org/freedesktop/login1"
#define DBUS_INTERFACE   "org.freedesktop.login1.Manager"

//code to determine local IP addr is taken from StackOverflow https://stackoverflow.com/a/59025254
std::string OptionsPage::get_ip_addr() {
    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    sockaddr_in loopback{};

    if (sock == -1) {
        return "Could not socket!";
    }

    std::memset(&loopback, 0, sizeof(loopback));
    loopback.sin_family = AF_INET;
    loopback.sin_addr.s_addr = 1337;   // can be any IP address
    loopback.sin_port = htons(9);      // using debug port

    if (connect(sock, reinterpret_cast<sockaddr *>(&loopback), sizeof(loopback)) == -1) {
        close(sock);
        return "Could not connect";
    }

    socklen_t addrlen = sizeof(loopback);
    if (getsockname(sock, reinterpret_cast<sockaddr *>(&loopback), &addrlen) == -1) {
        close(sock);
        return "Could not getsockname";
    }

    close(sock);

    char buf[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &loopback.sin_addr, buf, INET_ADDRSTRLEN) == nullptr) {
        return "Could not inet_ntop";
    } else {
        spdlog::info("OptionsPage::get_local_ip(): determined local ip address: {0}", buf);
        return std::string{buf};
    }
}

std::string OptionsPage::get_system_uptime() {
    std::string uptime = "< ERROR >";
    double uptime_seconds;
    if (std::ifstream("/proc/uptime", std::ios::in) >> uptime_seconds) {
        int secs = static_cast<int>(uptime_seconds);

        uptime = to_time_string(secs);
    }
    return uptime;
}

std::string OptionsPage::get_program_uptime() const {
    auto now = std::time(nullptr);
    auto diff = static_cast<int>(std::difftime(now, _model.program_start_time));
    return to_time_string(diff);
}

std::string OptionsPage::to_time_string(int secs) {
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

void OptionsPage::trigger_reboot() {
    try {
        auto proxy = sdbus::createProxy(DBUS_DESTINATION, DBUS_OBJPATH);
        auto method = proxy->createMethodCall(DBUS_INTERFACE, "Reboot");
        method << false;
        auto reply = proxy->callMethod(method);
    }catch(const sdbus::Error& e) {
        spdlog::error("OptionsPage::trigger_reboot(): failed with {0}", e.getMessage());
    }
}

void OptionsPage::trigger_shutdown() {
    try {
        auto proxy = sdbus::createProxy(DBUS_DESTINATION, DBUS_OBJPATH);
        auto method = proxy->createMethodCall(DBUS_INTERFACE, "PowerOff");
        method << false;
        auto reply = proxy->callMethod(method);
    }catch(const sdbus::Error& e){
        spdlog::error("OptionsPage::trigger_shutdown(): failed with {0}", e.getMessage());
    }
}

OptionsPage::OptionsPage(std::shared_ptr<StateController> &ctrl)
        : BasePage(OPTIONS, ctrl) {
    _model.local_ip = get_ip_addr();
    _model.system_uptime = get_system_uptime();
    _model.program_start_time = std::time(nullptr);
    _model.program_uptime = get_program_uptime();
}

OptionsPage::~OptionsPage() = default;

void OptionsPage::enter_page(PAGES origin, void *payload) {
    update_time();
    spdlog::info("OptionsPage::enter_page(): from origin {0}", origin);
}

void *OptionsPage::leave_page(PAGES destination) {
    spdlog::info("OptionsPage::leave_page(): to destination {0}", destination);
    return nullptr;
}

void OptionsPage::update_time() {
    BasePage::update_time();
    _model.system_uptime = get_system_uptime();
    _model.program_uptime = get_program_uptime();
}

void OptionsPage::handle_enter_key() {
    trigger_reboot();
}

void OptionsPage::handle_wheel_input(int delta) {
    //NOP
}

void OptionsPage::render(std::unique_ptr<Renderer> &renderer) {
    this->render_time(renderer);
    renderer->render_text(10, 40, "IPv4:\t" + _model.local_ip, Renderer::LARGE, Renderer::LEFT);
    renderer->render_text(10, 70,  "Sys. Up.: " + _model.system_uptime, Renderer::LARGE, Renderer::LEFT);
    renderer->render_text(10, 100, "Prg. Up.: " + _model.program_uptime, Renderer::LARGE, Renderer::LEFT);
    renderer->render_text(10, 130, "Shutdown", Renderer::LARGE, Renderer::LEFT);
    renderer->render_text(10, 160, "Reboot", Renderer::LARGE, Renderer::LEFT);
}