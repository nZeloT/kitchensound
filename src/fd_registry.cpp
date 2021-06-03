#include "kitchensound/fd_registry.h"

#include <unistd.h>
#include <sys/epoll.h>

#include <unordered_map>
#include <set>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#define MAX_EVENTS 20

struct FdRegistry::Impl {
    Impl()
        : _triggers{}, _epoll_fd{}, _cnt{}, _events{}, _fds_to_remove{} {

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

    void flagFdFroRemoval(int fd) {
        SPDLOG_INFO("FD {} flagged for removal from epoll.", fd);
        _fds_to_remove.insert(fd);
    }

    void removeFd(int fd) {
        if(_triggers.erase(fd)) {
            SPDLOG_INFO("Removing fd from epoll -> {}", fd);
            if(epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr)) {
                SPDLOG_ERROR("Received error while removing FD from epoll (errno) => {}", errno);
                switch(errno){
                    case EBADF: SPDLOG_ERROR("errno indicates EBADF => supplied fd is not valid"); break;
                    case EEXIST: SPDLOG_ERROR("errno indicates EEXIST => fd already registered"); break;
                    case ENOENT: SPDLOG_ERROR("errno indicates ENOENT => fd not registered (anymore?)"); break;
                    case ENOMEM: SPDLOG_ERROR("errno indicates ENOMEM => no memory to execute"); break;
                    case ENOSPC: SPDLOG_ERROR("errno indicated ENOSPC => breached max user watches"); break;
                    case EPERM: SPDLOG_ERROR("errno indicates EPERM => fd does not suport epoll"); break;
                    default: SPDLOG_ERROR("errno indicates another error"); break;
                }
                throw std::runtime_error{"Failed to remove fd from epoll!"};
            }
        }else {
            SPDLOG_WARN("Didn't remove unknown fd -> {}", fd);
        }
    }

    void softRemoveFd(int fd) {
        //fd was already removed from epoll; only housekeeping
        SPDLOG_INFO("Soft removing FD; make sure it's already removed from epoll (e.g. via close) => {}", fd);
        _triggers.erase(fd);
    }

    void wait() {
        auto evt_cnt = epoll_wait(_epoll_fd, _events, MAX_EVENTS, 10000);
        if(evt_cnt == -1){
            if(errno == EINTR){
                SPDLOG_WARN("epoll wait was interrupted!");
                return;
            }else{
                throw std::runtime_error{"Received unrecoverable epoll error!"};
            }
        }

        for(_cnt = 0; _cnt < evt_cnt; ++_cnt) {
            auto& e = _events[_cnt];
            SPDLOG_DEBUG("Received event for fd -> {}", e.data.fd);

            //it can happen that the fd we received an event for was removed from here while processing an event for another fd, skipping it in this case
            auto cb = _triggers.find(e.data.fd);
            if(cb != std::end(_triggers)) {
                //we don't read here and leave this to the respective fd owners
                _triggers[e.data.fd](e.data.fd, e.events);
            }
        }

        //remove flagged fd's to prevent epoll issues when trying to remove fd's which have events beeing processed
        for(auto it = std::begin(_fds_to_remove); it != std::end(_fds_to_remove); ++it) {
            removeFd(*it);
        }
        _fds_to_remove.clear();
    }

    epoll_event _events[MAX_EVENTS];
    int _epoll_fd, _cnt;
    std::set<int> _fds_to_remove;
    std::unordered_map<int, std::function<void(int, uint32_t)>> _triggers;
};


FdRegistry::FdRegistry()
    : _impl{std::make_unique<Impl>()} {}


FdRegistry::~FdRegistry() = default;


void FdRegistry::addFd(int fd, std::function<void(int, uint32_t)> cb, uint32_t epoll_evt_flags) {
    _impl->addFd(fd, std::move(cb), epoll_evt_flags);
}

void FdRegistry::removeFd(int fd) {
    _impl->flagFdFroRemoval(fd);
}

void FdRegistry::softRemoveFd(int fd) {
    _impl->softRemoveFd(fd);
}

void FdRegistry::wait() {
    _impl->wait();
}