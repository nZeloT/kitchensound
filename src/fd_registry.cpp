#include "kitchensound/fd_registry.h"

#include <unistd.h>
#include <sys/epoll.h>

#include <unordered_map>

#include <spdlog/spdlog.h>

#define MAX_EVENTS 20

struct FdRegistry::Impl {
    Impl()
        : _triggers{}, _epoll_fd{}, _cnt{}, _events{} {

        _epoll_fd = epoll_create1(0);
        if(_epoll_fd == -1)
            std::runtime_error{"Failed to create epoll fd!"};
    };

    ~Impl() {
        close(_epoll_fd);
    };

    void addFd(int fd, std::function<void(int, uint32_t)> cb_trigger, uint32_t epoll_evt_flags) {
        auto success = _triggers.insert(std::make_pair(fd, std::move(cb_trigger)));

        if(!success.second)
            throw std::runtime_error{"Tried to add the same fd twice!"};

        epoll_event evt{};
        evt.events = epoll_evt_flags;
        evt.data.fd = fd;

        SPDLOG_DEBUG("Adding fd to epoll -> {}", fd);

        if(epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &evt))
            throw std::runtime_error{"Failed to add fd to epoll!"};
    }

    void removeFd(int fd) {
        if(_triggers.erase(fd)) {
            SPDLOG_DEBUG("Removing fd from epoll -> {}", fd);
            if(epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr))
                throw std::runtime_error{"Failed to remove fd from epoll!"};
        }else {
            SPDLOG_WARN("Didn't remove unknown fd -> {}", fd);
        }
    }

    void wait() {
        auto evt_cnt = epoll_wait(_epoll_fd, _events, MAX_EVENTS, 10000);
        if(evt_cnt == -1){
            if(errno == EINTR){
                SPDLOG_INFO("epoll wait was interrupted!");
                return;
            }else{
                throw std::runtime_error{"Received unrecoverable epoll error!"};
            }
        }

        for(_cnt = 0; _cnt < evt_cnt; ++_cnt) {
            auto& e = _events[_cnt];
            SPDLOG_DEBUG("Received event for fd -> {}", e.data.fd);
            //we don't read here and leave this to the respective fd owners
            _triggers[e.data.fd](e.data.fd, e.events);
        }
    }

    epoll_event _events[MAX_EVENTS];
    int _epoll_fd, _cnt;
    std::unordered_map<int, std::function<void(int, uint32_t)>> _triggers;
};


FdRegistry::FdRegistry()
    : _impl{std::make_unique<Impl>()} {}


FdRegistry::~FdRegistry() = default;


void FdRegistry::addFd(int fd, std::function<void(int, uint32_t)> cb, uint32_t epoll_evt_flags) {
    _impl->addFd(fd, std::move(cb), epoll_evt_flags);
}

void FdRegistry::removeFd(int fd) {
    _impl->removeFd(fd);
}

void FdRegistry::wait() {
    _impl->wait();
}