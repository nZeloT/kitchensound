#include "kitchensound/mpd_controller.h"

#include <sstream>

#include <spdlog/spdlog.h>
#include <mpd/client.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/timer.h"
#include "kitchensound/fd_registry.h"

static std::string read_tag(const mpd_song *song, const mpd_tag_type type) {
    uint i = 0;
    const char *value;
    std::ostringstream tag_value;
    while ((value = mpd_song_get_tag(song, type, i++)) != nullptr)
        tag_value << value;
    return tag_value.str();
}

struct MPDController::Impl {
    explicit Impl(Configuration::MPDConfig conf, std::unique_ptr<FdRegistry>& fdr)
    : _mpd_config{std::move(conf)}, _cb_metadata{[](const std::string&){}},
      _connection{nullptr}, _current_meta{},
      _polling_timer{std::make_unique<Timer>(fdr, "MpdController MPD update", MPD_POLLING_DELAY, true, [this](){
          this->poll_metadata();
      })} {};

    ~Impl() {
        stop_playback();
    };

    void playback_stream(const std::string& stream_url){
        create_mpd_connection();
        SPDLOG_INFO("Starting stream -> {0}", stream_url);

        mpd_run_add(_connection, stream_url.c_str());
        auto err = check_for_error();

        if(!err) {
            mpd_run_play(_connection);
            err = check_for_error();
        }

        if(err)
            throw std::runtime_error("Error occurred while trying to play stream!");

        _polling_timer->reset_timer();
        close_connection();
    }

    void stop_playback(){
        SPDLOG_INFO("Stopping MPD playback");
        create_mpd_connection();
        mpd_run_stop(_connection);
        auto err = check_for_error();
        if (!err) {
            mpd_run_clear(_connection);
            err = check_for_error();
        }
        if (err)
            throw std::runtime_error("Error occurred while trying to clear queue!");

        _polling_timer->stop();
       close_connection();
    }

    void poll_metadata() {
        create_mpd_connection();
        auto status = mpd_run_status(_connection);
        check_for_error();

        if (status != nullptr && mpd_status_get_state(status) == MPD_STATE_PLAY) {
            auto song = mpd_run_current_song(_connection);
            if (!check_for_error()) {
                auto song_title = read_tag(song, MPD_TAG_TITLE);
                if(_current_meta != song_title) {
                    SPDLOG_INFO("Metadata changed -> {0}", song_title);
                    _cb_metadata(song_title);
                    _current_meta = song_title;
                }

                mpd_song_free(song);
            }
        }

        close_connection();
    }

    bool check_for_error(bool after_reset = false) {
        if(_connection == nullptr)
            throw std::runtime_error{"MPD Connection is null! This should not happen!"};

        mpd_error err_code;
        bool encountered_error = false;

        if((err_code = mpd_connection_get_error(_connection)) != MPD_ERROR_SUCCESS) {
            SPDLOG_ERROR("mpd connection error -> {0}", mpd_connection_get_error_message(_connection));
            encountered_error = true;
        }

        if(err_code == MPD_ERROR_OOM || err_code == MPD_ERROR_TIMEOUT
        || err_code == MPD_ERROR_SYSTEM || err_code == MPD_ERROR_RESOLVER
        || err_code == MPD_ERROR_CLOSED){
            if(!after_reset)
                create_mpd_connection();
            else
                throw std::runtime_error{"Can't establish MPD connection!"};
        }

        return encountered_error;
    }

    void create_mpd_connection() {
        if(_connection != nullptr)
            mpd_connection_free(_connection);

        _connection = mpd_connection_new(_mpd_config.address.c_str(), _mpd_config.port, 30000);
        check_for_error(true);

        SPDLOG_DEBUG("MPD connection created.");
    }

    void close_connection() {
        mpd_connection_free(_connection);
        _connection = nullptr;
        SPDLOG_DEBUG("MPD connection closed.");
    }

    void set_metadata_callback(std::function<void(const std::string&)> cb){
        _cb_metadata = std::move(cb);
    }


    mpd_connection* _connection;
    const Configuration::MPDConfig _mpd_config;
    std::unique_ptr<Timer> _polling_timer;
    std::string _current_meta;
    std::function<void(const std::string&)> _cb_metadata;
};

MPDController::MPDController(Configuration::MPDConfig config, std::unique_ptr<FdRegistry>& tm)
        : _impl{std::make_unique<MPDController::Impl>(std::move(config), tm)} {}

MPDController::~MPDController() = default;

void MPDController::playback_stream(const std::string &stream_url) {
    _impl->playback_stream(stream_url);
}

void MPDController::stop_playback() {
    _impl->stop_playback();
}

void MPDController::force_metadata_update() {
    _impl->poll_metadata();
}

void MPDController::set_metadata_callback(std::function<void(const std::string &)> update_handler) {
    _impl->set_metadata_callback(std::move(update_handler));
}
