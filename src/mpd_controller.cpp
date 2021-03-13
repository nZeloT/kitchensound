#include "kitchensound/mpd_controller.h"

#include <sstream>

#include <spdlog/spdlog.h>
#include <mpd/client.h>

#include "kitchensound/timeouts.h"
#include "kitchensound/timer.h"

static std::string read_tag(const mpd_song *song, const mpd_tag_type type) {
    uint i = 0;
    const char *value;
    std::ostringstream tag_value;
    while ((value = mpd_song_get_tag(song, type, i++)) != nullptr)
        tag_value << value;
    return tag_value.str();
}

struct MPDController::Impl {
    explicit Impl(Configuration::MPDConfig conf)
    : _mpd_config{std::move(conf)}, _cb_metadata{[](const std::string&){}},
      _connection{nullptr}, _is_polling{false}, _current_meta{},
      _polling_timer{MPD_POLLING_DELAY, false, [this](){
          this->poll_metadata();
      }} {
        reset_connection();
        mpd_connection_set_keepalive(_connection, true);
    };

    ~Impl() {
        close_connection();
    };

    void playback_stream(const std::string& stream_url){
        SPDLOG_INFO("Starting stream -> {0}", stream_url);

        mpd_run_add(_connection, stream_url.c_str());
        auto err = check_for_error();

        if(!err) {
            mpd_run_play(_connection);
            err = check_for_error();
        }

        if(err)
            throw std::runtime_error("Error occurred while trying to play stream!");

        _polling_timer.reset();
        _is_polling = true;
    }

    void stop_playback(){
        SPDLOG_INFO("Stopping MPD playback");

        mpd_run_stop(_connection);
        auto err = check_for_error();
        if (!err) {
            mpd_run_clear(_connection);
            err = check_for_error();
        }
        if (err)
            throw std::runtime_error("Error occurred while trying to clear queue!");

        _is_polling = false;
    }

    void update(long ms_delta_time){
        if(_is_polling)
            _polling_timer.update(ms_delta_time);
    }

    void poll_metadata() {
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

        if(_is_polling)
            _polling_timer.reset();
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
                reset_connection();
            else
                throw std::runtime_error{"Can't establish MPD connection!"};
        }

        return encountered_error;
    }

    void reset_connection() {
        if(_connection != nullptr)
            close_connection();

        _connection = mpd_connection_new(_mpd_config.address.c_str(), _mpd_config.port, 30000);
        check_for_error(true);

        SPDLOG_INFO("Connected to MPD.");
    }

    void close_connection() {
        mpd_run_stop(_connection);
        mpd_run_clear(_connection);
        mpd_connection_free(_connection);
        SPDLOG_INFO("Disconnected from MPD.");
        _connection = nullptr;
    }

    void set_metadata_callback(std::function<void(const std::string&)> cb){
        _cb_metadata = std::move(cb);
    }


    mpd_connection* _connection;
    bool _is_polling;
    const Configuration::MPDConfig _mpd_config;
    Timer _polling_timer;
    std::string _current_meta;
    std::function<void(const std::string&)> _cb_metadata;
};

MPDController::MPDController(Configuration::MPDConfig config)
        : _impl{std::make_unique<MPDController::Impl>(std::move(config))} {}

MPDController::~MPDController() = default;

void MPDController::update(long ms_delta_time) {
    _impl->update(ms_delta_time);
}

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
