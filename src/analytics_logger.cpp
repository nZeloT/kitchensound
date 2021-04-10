#include "kitchensound/analytics_logger.h"

#include <chrono>

#include <spdlog/spdlog.h>

#include "kitchensound/network_controller.h"

#include "generated/analytics_protocol_generated.h"

struct AnalyticsLogger::Impl {
    Impl(std::unique_ptr<NetworkController> &net, Configuration::AnalyticsConfig conf)
            : _net{net}, _conf{std::move(conf)} {}

    ~Impl() = default;

    void log_page_change(PAGES origin, PAGES destination) const {
        if (!_conf.enabled)
            return;

        flatbuffers::FlatBufferBuilder fbb{};
        auto pageChange = CreatePageChange(fbb, map_page(origin), map_page(destination));
        auto analyticsMessage = CreateAnalyticsMessageDirect(fbb, "Kitchensound", get_timestamp_of_now(),
                                                             AnalyticsMessageType::PageChange, pageChange.Union());
        fbb.Finish(analyticsMessage);
        send_analytics_message(fbb.GetBufferPointer(), fbb.GetSize());
    }

    void log_playback_change(PLAYBACK_SOURCE source, std::string const &name, bool started) const {
        if (!_conf.enabled)
            return;

        flatbuffers::FlatBufferBuilder fbb{};
        auto playbackChange = CreatePlaybackChangeDirect(fbb, started, map_playback_source(source), name.c_str());
        auto msg = CreateAnalyticsMessageDirect(fbb, "Kitchensound", get_timestamp_of_now(),
                                                AnalyticsMessageType::PlaybackChange, playbackChange.Union());
        fbb.Finish(msg);
        send_analytics_message(fbb.GetBufferPointer(), fbb.GetSize());
    }

    void log_playback_song_change(std::string const &raw_meta, std::string const &title, std::string const &artist,
                                  std::string const &album) const {
        if (!_conf.enabled)
            return;

        flatbuffers::FlatBufferBuilder fbb{};
        auto playbackSongChange = CreatePlaybackSongChangeDirect(fbb, raw_meta.c_str(), title.c_str(), artist.c_str(),
                                                                 album.c_str());
        auto msg = CreateAnalyticsMessageDirect(fbb, "Kitchensound", get_timestamp_of_now(),
                                                AnalyticsMessageType::PlaybackSongChange, playbackSongChange.Union());
        fbb.Finish(msg);
        send_analytics_message(fbb.GetBufferPointer(), fbb.GetSize());
    }

    static uint64_t get_timestamp_of_now() {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }

    static PlaybackSource map_playback_source(PLAYBACK_SOURCE p) {
        switch (p) {
            case SNAPCAST:
                return PlaybackSource::SNAPCAST;
            case RADIO_STREAM:
                return PlaybackSource::RADIO;
            case BLUETOOTH:
                return PlaybackSource::BLUETOOTH;
            default:
                throw std::runtime_error{"Failed to map to PlaybackSource in AnalyticsLogger!"};
        }
    }

    static Page map_page(PAGES p) {
        switch (p) {
            case INACTIVE:
                return Page::INACTIVE;
            case LOADING:
                return Page::LOADING;
            case STREAM_SELECTION:
                return Page::RADIO_SELECTION;
            case STREAM_PLAYING:
                return Page::RADIO_PLAYING;
            case BT_PLAYING:
                return Page::BT_PLAYING;
            case OPTIONS:
                return Page::OPTIONS;
            case MENU_SELECTION:
                return Page::MENU_SELECTION;
            case SNAPCAST_PLAYING:
                return Page::SNAPCAST_PLAYING;
            default:
                throw std::runtime_error{"Failed to map page in analytics logger!"};
        }
    }

    void send_analytics_message(uint8_t *buffer, size_t size) const {
        SPDLOG_INFO("Sending analytics message with size => {}", size);
        _net->add_request(_conf.dest_host, HTTP_METHOD::POST, [this](auto _1, auto success, auto _2) {
            if (success) {
                SPDLOG_INFO("Successful Analytics message sent.");
            } else {
                SPDLOG_WARN("Failed to send analytics message -> {}", this->_conf.dest_host);
            }
        }, buffer, size);
    }

    std::unique_ptr<NetworkController> &_net;
    Configuration::AnalyticsConfig _conf;
};

AnalyticsLogger::AnalyticsLogger(std::unique_ptr<NetworkController> &net, Configuration::AnalyticsConfig conf)
        : _impl{std::make_unique<Impl>(net, std::move(conf))} {}

AnalyticsLogger::~AnalyticsLogger() = default;

void AnalyticsLogger::log_page_change(PAGES origin, PAGES destination) {
    _impl->log_page_change(origin, destination);
}

void AnalyticsLogger::log_playback_change(PLAYBACK_SOURCE source, const std::string &name, bool started) {
    _impl->log_playback_change(source, name, started);
}

void AnalyticsLogger::log_playback_song_change(std::string const &raw_meta, std::string const &title,
                                               std::string const &artist,
                                               std::string const &album) {
    _impl->log_playback_song_change(raw_meta, title, artist, album);
}