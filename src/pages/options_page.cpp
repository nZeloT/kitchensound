#include "kitchensound/pages/options_page.h"

#include <spdlog/spdlog.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "kitchensound/renderer.h"

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

    if (connect(sock, reinterpret_cast<sockaddr*>(&loopback), sizeof(loopback)) == -1) {
        close(sock);
        return "Could not connect";
    }

    socklen_t addrlen = sizeof(loopback);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&loopback), &addrlen) == -1) {
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

OptionsPage::OptionsPage(std::shared_ptr<StateController>& ctrl)
    : BasePage(OPTIONS, ctrl){
    _model.local_ip = std::move(get_ip_addr());
    _model.startup_time = std::time(nullptr);
}

OptionsPage::~OptionsPage() = default;

void OptionsPage::enter_page(PAGES origin, void* payload) {
    spdlog::info("OptionsPage::enter_page(): from origin {0}", origin);
}

void* OptionsPage::leave_page(PAGES destination) {
    spdlog::info("OptionsPage::leave_page(): to destination {0}", destination);
    return nullptr;
}

void OptionsPage::handle_enter_key() {
    //NOP
}

void OptionsPage::handle_wheel_input(int delta) {
    //NOP
}

void OptionsPage::render(std::unique_ptr<Renderer>& renderer) {
    this->render_time(renderer);
    renderer->render_text_small(160, 35, _model.local_ip);
}