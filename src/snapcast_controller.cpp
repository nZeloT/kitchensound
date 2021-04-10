#include "kitchensound/snapcast_controller.h"

#include <cstdlib>
#include <csignal>
#include <unistd.h>
#include <sys/epoll.h>
#include <vector>

#include <spdlog/spdlog.h>

#include "kitchensound/fd_registry.h"

#define BUFFER_SIZE 131072

struct SnapcastController::Impl {
    Impl(std::unique_ptr<FdRegistry> &fdreg, Configuration::SnapcastConfig config)
            : _fdreg{fdreg}, _is_running{false}, _pid{-1}, _buffer{new char[BUFFER_SIZE]},
            _config{std::move(config)}, _metadata_pipe{} {};

    ~Impl() = default;

    void start_snapclient() {
        //using fork(), pipe() and dup2() to start the snapclient and listen to its stderr (metadata)
        if (_is_running)
            return;

        if(::pipe(_metadata_pipe)){
            throw std::runtime_error{"Failed to create pipe() for snapclient."};
        }

        _pid = ::fork();
        if (_pid < 0)
            throw std::runtime_error{"Failed to  fork() when starting snapclient!"};
        else if (_pid == 0) {
            //child process; here duplicate the std::cerr to the pipe

            //first close the read end on this side of the pipe
            close(_metadata_pipe[0]);

            if(_metadata_pipe[1] != STDERR_FILENO){
                if(dup2(_metadata_pipe[1], STDERR_FILENO) != STDERR_FILENO)
                    throw std::runtime_error{"Failed to duplicate std::cerr to the pipe!"}; //TODO check if this is the right approach here

                close(_metadata_pipe[1]);
            }


            if(::execl(_config.bin.c_str(), _config.bin.c_str(),
                      "-h", _config.host.c_str(),
                      "-p", std::to_string(_config.port).c_str(),
                      "-s", _config.alsa_pcm.c_str(),
                      "-e") < 0) {

                throw std::runtime_error{"Failed to execute snapclient!"};
            }

        }else{
            //parent process continues here

            //first close the write channel on this side of the pipe
            ::close(_metadata_pipe[1]);

            _fdreg->addFd(_metadata_pipe[0], [this](auto _1, auto _2) {
                this->read_metadata_pipe();
            }, EPOLLIN);

            _is_running = true;
        }
    }

    void stop_snapclient() {
        if (!_is_running)
            return;

        _fdreg->removeFd(_metadata_pipe[0]);

        //use kill to stop the running snapclient
        ::kill(_pid, SIGTERM);

        _is_running = false;
    }

    void read_metadata_pipe() {
        auto size = ::read(_metadata_pipe[0], _buffer.get(), BUFFER_SIZE);
        if(size < 0)
            throw std::runtime_error{"Failed to read new snapclient metadata!"};

        auto meta = std::string(_buffer.get(), size);

        SPDLOG_INFO("Snapcast Meta String -> {}", meta);

        //TODO maybe there is some parsing necessary
    }

    pid_t _pid;
    bool _is_running;
    int _metadata_pipe[2];
    std::unique_ptr<char[]> _buffer;

    Configuration::SnapcastConfig _config;
    std::unique_ptr<FdRegistry> &_fdreg;

};

SnapcastController::SnapcastController(std::unique_ptr<FdRegistry> &fdreg, Configuration::SnapcastConfig config)
        : _impl{std::make_unique<Impl>(fdreg, std::move(config))} {};

SnapcastController::~SnapcastController() = default;


void SnapcastController::start_snapclient_service() {
    _impl->start_snapclient();
}

void SnapcastController::stop_snapclient_service() {
    _impl->stop_snapclient();
}