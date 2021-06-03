#include "kitchensound/snapcast_controller.h"

#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/song.h"
#include "kitchensound/fd_registry.h"
#include "kitchensound/mpd_controller.h"

#define BUFFER_SIZE 131072

struct SnapcastController::Impl {
    Impl(std::unique_ptr<FdRegistry> &fdreg, std::unique_ptr<AnalyticsLogger>& analytics, Configuration::SnapcastConfig config)
            : _pid{-1}, _is_running{false}, _error_pipe{}, _stdout_pipe{}, _buffer{new char[BUFFER_SIZE]}, _buffer2{new char[BUFFER_SIZE]},
              _config{std::move(config)}, _fdreg{fdreg}, _mpd{std::make_unique<ExtendedMPDController>(fdreg, analytics, _config.mpd_feed)},
              _cb{[](auto& _){}}
    {
        _mpd->set_metadata_callback([this](auto& song){
           this->received_new_song(song);
        });
        _mpd->set_tags_to_read(MPD_TAG::TITLE | MPD_TAG::ARTIST | MPD_TAG::ALBUM);

        SPDLOG_INFO("Found snapcast configuration -> {}; {}:{}; {}; {}:{}", _config.bin, _config.host, _config.port, _config.has_mpd_feed, _config.mpd_feed.address, _config.mpd_feed.port);
    };

    ~Impl() = default;

    void start_snapclient() {
        //using fork(), pipe() and dup2() to start the snapclient and listen to its stderr (metadata)
        if (_is_running)
            return;

        SPDLOG_INFO("Creating pipes");
        if(::pipe(_error_pipe)){
            throw std::runtime_error{"Failed to create stderr pipe() for snapclient."};
        }
        if(::pipe(_stdout_pipe)){
            throw std::runtime_error{"Failed to create stdout pipe() for snapclient."};
        }

        SPDLOG_INFO("Forking ...");
        _pid = ::fork();
        if (_pid < 0)
            throw std::runtime_error{"Failed to  fork() when starting snapclient!"};
        else if (_pid == 0) {
            SPDLOG_INFO("Hello from the snapcast child");
            //child process; here duplicate the std::cerr to the pipe

            //first close the read end on this side of the pipe
            close(_error_pipe[0]);
            close(_stdout_pipe[0]);

            if(_error_pipe[1] != STDERR_FILENO){
                if(dup2(_error_pipe[1], STDERR_FILENO) != STDERR_FILENO)
                    throw std::runtime_error{"Failed to duplicate std::cerr to the pipe!"}; //TODO check if this is the right approach here

                close(_error_pipe[1]);
            }

            if(_stdout_pipe[1] != STDOUT_FILENO){
                if(dup2(_stdout_pipe[1], STDOUT_FILENO) != STDOUT_FILENO)
                    throw std::runtime_error{"Failed to duplicate std::cout to the pipe!"}; //TODO check if this is the right approach here

                close(_stdout_pipe[1]);
            }

            SPDLOG_INFO("Launching snapclient with host => {}; port => {}; soundcard => {}", _config.host, _config.port, _config.alsa_pcm);

            if(::execl(_config.bin.c_str(), _config.bin.c_str(),
                      "-h", _config.host.c_str(),
                      "-p", std::to_string(_config.port).c_str(),
                      "-s", _config.alsa_pcm.c_str()) < 0) {
                SPDLOG_ERROR("Failed to execute snapclient!");
                ::_exit(-1);
                throw std::runtime_error{"Failed to execute snapclient!"};
            }

        }else{
            SPDLOG_INFO("Parent continues");
            //parent process continues here

            //first close the write channel on this side of the pipe
            ::close(_error_pipe[1]);
            ::close(_stdout_pipe[1]);

            _fdreg->addFd(_error_pipe[0], [this](auto _1, auto _2) {
                this->read_error_pipe();
            }, EPOLLIN);
            _fdreg->addFd(_stdout_pipe[0], [this](auto _1, auto _2) {
                this->read_stdout_pipe();
            }, EPOLLIN);

            _is_running = true;
            _mpd->start_polling();
        }
    }

    void stop_snapclient() {
        if (!_is_running)
            return;
        SPDLOG_INFO("Stopping fork");

        //use kill to stop the running snapclient
        ::kill(_pid, SIGTERM);
        SPDLOG_INFO("Sent SIGTERM to fork");

        ::close(_error_pipe[0]); //<- close auto removes from epoll
        ::close(_stdout_pipe[0]);

        _fdreg->softRemoveFd(_error_pipe[0]); //<-- only soft remove as they are already removed from epoll
        _fdreg->softRemoveFd(_stdout_pipe[0]);

        _mpd->stop_polling();
        _is_running = false;
    }

    void read_error_pipe() {
        auto size = ::read(_error_pipe[0], _buffer.get(), BUFFER_SIZE);
        if(size < 0)
            throw std::runtime_error{"Failed to read new snapclient error!"};

        if(size > 0) {
            auto error = std::string(_buffer.get(), size);
            SPDLOG_ERROR("Snapcast Error String -> {}", error);
        }
    }

    void read_stdout_pipe() {
        auto size = ::read(_stdout_pipe[0], _buffer2.get(), BUFFER_SIZE);
        if(size < 0)
            throw std::runtime_error{"Failed to read new snapclient stdout!"};

        if(size > 0) {
            auto stdout = std::string(_buffer2.get(), size);
            SPDLOG_DEBUG("Snapcast Stdout String -> {}", stdout);
        }
    }

    void received_new_song(Song const& song) {
        SPDLOG_INFO("Received new song => {}", song.to_string());
        _cb(song);
    }

    void set_metadata_callback(std::function<void(Song const&)> cb) {
        _cb = std::move(cb);
    }

    pid_t _pid;
    bool _is_running;
    int _error_pipe[2];
    int _stdout_pipe[2];
    std::unique_ptr<char[]> _buffer;
    std::unique_ptr<char[]> _buffer2;

    Configuration::SnapcastConfig _config;
    std::unique_ptr<FdRegistry> &_fdreg;
    std::unique_ptr<ExtendedMPDController> _mpd;
    std::function<void(Song const&)> _cb;

};

SnapcastController::SnapcastController(std::unique_ptr<FdRegistry> &fdreg, std::unique_ptr<AnalyticsLogger>& analytics, Configuration::SnapcastConfig config)
        : _impl{std::make_unique<Impl>(fdreg, analytics, std::move(config))} {};

SnapcastController::~SnapcastController() = default;


void SnapcastController::start_snapclient_service() {
    _impl->start_snapclient();
}

void SnapcastController::stop_snapclient_service() {
    _impl->stop_snapclient();
}

void SnapcastController::set_metadata_cb(std::function<void(Song const&)> cb) {
    _impl->set_metadata_callback(std::move(cb));
}