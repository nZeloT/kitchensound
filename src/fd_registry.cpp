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
        : _active_triggers{}, _epoll_fd{}, _cnt{}, _events{}, _fds_to_remove{}, _fds_to_add{} {

        _epoll_fd = epoll_create1(0);
        if(_epoll_fd == -1)
            std::runtime_error{"Failed to create epoll fd!"};
    };

    ~Impl() {
        SPDLOG_DEBUG("Closing epoll fd.");
        close(_epoll_fd);
    };

    void addFd(int fd, std::function<void(int, uint32_t)> cb_trigger, uint32_t epoll_evt_flags) {
        auto fd_is_known = _active_triggers.find(fd) != _active_triggers.end();

        if(fd_is_known) {
            auto flagged_for_removal = _fds_to_remove.find(fd) != _fds_to_remove.end();

            auto already_flagged_for_add = false;
            if(flagged_for_removal) {
                SPDLOG_INFO("Adding fd {} for later update as it's already known, but flagged for removal.", fd);
                already_flagged_for_add = !_fds_to_add.insert(std::make_pair(fd, std::make_pair(std::move(cb_trigger), epoll_evt_flags))).second;
            }

            if(!flagged_for_removal || already_flagged_for_add)
                throw std::runtime_error{"Tried to add the same fd twice!"};

        }else{
            _active_triggers.insert(std::make_pair(fd, std::move(cb_trigger)));

            epoll_event evt{};
            evt.events = epoll_evt_flags;
            evt.data.fd = fd;

            SPDLOG_DEBUG("Adding fd to epoll -> {}", fd);

            if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &evt))
                throw std::runtime_error{"Failed to add fd to epoll!"};
        }
    }

    void flagFdForRemoval(int fd) {
        SPDLOG_INFO("FD {} flagged for removal from epoll.", fd);
        _fds_to_remove.insert(fd);
    }

    void removeFd(int fd) {
        if(_active_triggers.erase(fd)) {
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
        _active_triggers.erase(fd);
    }

    void delayedAddFd(std::pair<int, std::pair<std::function<void(int, uint32_t)>, uint32_t>> to_add) {
        //we came here because we tried to add a Fd which was already part of Epoll and therefore deferred the addition
        // until now when event processing is done and changes to epoll can safely be done
        removeFd(to_add.first);
        addFd(to_add.first, std::move(to_add.second.first), to_add.second.second);
        _fds_to_remove.erase(to_add.first);
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
            auto cb = _active_triggers.find(e.data.fd);
            if(cb != std::end(_active_triggers)) {
                //we don't read here and leave this to the respective fd owners
                _active_triggers[e.data.fd](e.data.fd, e.events);
            }
        }

        for(auto it = std::begin(_fds_to_add); it != std::end(_fds_to_add); ++it) {
            delayedAddFd(*it);
        }
        _fds_to_add.clear();

        //remove flagged fd's to prevent epoll issues when trying to remove fd's which have events beeing processed
        for(auto it = std::begin(_fds_to_remove); it != std::end(_fds_to_remove); ++it) {
            removeFd(*it);
        }
        _fds_to_remove.clear();
    }

    epoll_event _events[MAX_EVENTS];
    int _epoll_fd, _cnt;
    std::set<int> _fds_to_remove;
    std::unordered_map<int, std::pair<std::function<void(int, uint32_t)>, uint32_t>> _fds_to_add;
    std::unordered_map<int, std::function<void(int, uint32_t)>> _active_triggers;
};


FdRegistry::FdRegistry()
    : _impl{std::make_unique<Impl>()} {}


FdRegistry::~FdRegistry() {
    SPDLOG_DEBUG("FdRegistry dropped.");
}


void FdRegistry::addFd(int fd, std::function<void(int, uint32_t)> cb, uint32_t epoll_evt_flags) {
    _impl->addFd(fd, std::move(cb), epoll_evt_flags);
}

void FdRegistry::removeFd(int fd) {
    _impl->flagFdForRemoval(fd);
}

void FdRegistry::softRemoveFd(int fd) {
    _impl->softRemoveFd(fd);
}

void FdRegistry::wait() {
    _impl->wait();
}