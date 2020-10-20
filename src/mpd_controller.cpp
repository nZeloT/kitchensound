#include "kitchensound/mpd_controller.h"

#include <sstream>

#include <SDL.h>
#include <spdlog/spdlog.h>
#include <mpd/client.h>

#include "kitchensound/running.h"

mpd_connection *connect_to_mpd() {
    return mpd_connection_new("localhost", 6600, 30000);
}

bool check_for_error(mpd_connection *connection) {
    if (mpd_connection_get_error(connection) != MPD_ERROR_SUCCESS) {
        spdlog::error("mpd_connection_get_error(): {1}", mpd_connection_get_error_message(connection));
        return true;
    }
    return false;
}

static std::string read_tag(const mpd_song *song, const mpd_tag_type type) {
    uint i = 0;
    const char *value;
    std::ostringstream tag_value;
    while ((value = mpd_song_get_tag(song, type, i++)) != nullptr)
        tag_value << value;
    return tag_value.str();
}

static bool run_metadata_poller = true;

static int poll_metadata(void *ptr) {
    auto ctrl_ptr = reinterpret_cast<MPDController *>(ptr);

    spdlog::info("Entering MPDMetadataPoller");
    auto connection = connect_to_mpd();
    spdlog::info("MPDMetadataPoller connected!");

    while (running && run_metadata_poller) {
        spdlog::info("New poll");
        auto status = mpd_run_status(connection);
        if (mpd_status_get_state(status) == MPD_STATE_PLAY) {
            auto song = mpd_run_current_song(connection);
            if (check_for_error(connection)) {
                mpd_connection_free(connection);
                spdlog::error("Found erronous connection to MPD; Trying to reconnect;");
                connection = connect_to_mpd();
                if (check_for_error(connection)) {
                    running = false;
                    throw std::runtime_error("Failed to reconnect to MPD!");
                }
            }

            auto song_title = read_tag(song, MPD_TAG_TITLE);
            spdlog::info("New Title: {1}", song_title);
            ctrl_ptr->_update_handler(song_title);

            mpd_song_free(song);
        }

        SDL_Delay(5000); // poll every 5 sec
    }

    spdlog::info("Exiting MPDMetadataPoller");
    mpd_connection_free(connection);
    spdlog::info("MPDMetadataPolelr disconnected!");

    return 0;
}

MPDController::MPDController()
        : _update_handler(), _polling_thread{nullptr}, _initialized{false}, _connection{nullptr} {}

MPDController::~MPDController() {
    mpd_run_stop(_connection);
    mpd_run_clear(_connection);
    spdlog::info("Disconnecting from MPD!");
    mpd_connection_free(_connection);
    _connection = nullptr;
};

void MPDController::playback_stream(const std::string &stream_url) {
    spdlog::info("Starting stream");
    mpd_run_add(_connection, stream_url.c_str());
    if (_check_and_try())
        throw std::runtime_error("Error occurred while trying to add stream!");

    spdlog::info("Added new stream, starting playback");
    mpd_run_play(_connection);
    if (_check_and_try())
        throw std::runtime_error("Error occurred while trying to play stream!");

    run_metadata_poller = true;
    _start_polling();
}

void MPDController::stop_playback() {
    spdlog::info("Stopping playback");
    mpd_run_stop(_connection);
    if (_check_and_try())
        throw std::runtime_error("Error occurred while trying to add stop stream!");

    mpd_run_clear(_connection);
    if (_check_and_try())
        throw std::runtime_error("Error occurred while trying to clear queue!");

    spdlog::info("Stopping polling");
    _stop_polling();
}

bool MPDController::_check_and_try() {
    if (check_for_error(_connection)) {
        mpd_connection_free(_connection);
        _connection = connect_to_mpd();
        return check_for_error(_connection);
    }
    return false;
}

void MPDController::_start_polling() {
    if (_polling_thread != nullptr)
        throw std::runtime_error("Still have a running polling thread!");

    _polling_thread = SDL_CreateThread(poll_metadata, "MPDMetadataPoller", this);
    if (_polling_thread == nullptr) {
        spdlog::error("MPDMetadataPoller Thread creation failed!");
        running = false;
        throw std::runtime_error("Thread creation failed!");
    }
}

void MPDController::_stop_polling() {
    run_metadata_poller = false;
    if (_polling_thread != nullptr) {
        spdlog::warn("Polling thread != nullptr => waiting for it to stop");
        int thread_return;
        SDL_WaitThread(_polling_thread, &thread_return);
        _polling_thread = nullptr;
    }
}

void MPDController::init(std::function<void(const std::string &)> update_handler) {
    _get()._update_handler = std::move(update_handler);
    _get()._connection = connect_to_mpd();
    _get()._initialized = true;
    spdlog::info("Connected to MPD");
}
