#include "kitchensound/song_faver.h"

#include <unordered_map>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "kitchensound/network_controller.h"

#include "generated/song_like_protocol_generated.h"

#define REQUESTING_PARTY "Kitchensound"

struct SongFaver::Impl {
    Impl(std::unique_ptr<NetworkController> &net, Configuration::SongFaverConfig conf)
            : _net{net}, _conf{std::move(conf)}, _last_msg_id{0}, _callbacks{} {}

    void send_request(uint64_t msg_id, RequestAction a, SongSource const &source, Song const &s, std::function<void(uint64_t, SongState)> cb) {
        if (!_conf.enabled)
            cb(msg_id, SongState::DISABLED);

        flatbuffers::FlatBufferBuilder fbb{};
        auto song_info = CreateSongInfoDirect(fbb, s.raw_meta.c_str(), s.title.c_str(), s.artist.c_str(),
                                              s.album.c_str());
        auto song_source = CreateSongSourceInfoDirect(fbb, map_to_fbb_source_kind(source.kind), source.name.c_str());

        auto request = CreateRequestDirect(fbb, REQUESTING_PARTY, a, song_source, song_info);
        auto song_msg = CreateSongMessage(fbb, msg_id, MessagePayload::Request, request.Union());

        fbb.Finish(song_msg);
        _callbacks[msg_id] = std::move(cb);
        send_song_like_message(msg_id, fbb.GetBufferPointer(), fbb.GetSize());
    }

    void send_song_like_message(uint64_t msg_id, uint8_t* buffer, size_t size) {
        SPDLOG_INFO("Sending song_like message with size => {}", size);
        _net->add_request(_conf.dest_host, HTTP_METHOD::POST, [this,msg_id](auto _1, bool req_success, auto buf) {
            if(!req_success) {
                SPDLOG_INFO("song_like request failed with an error! Check connectivity and server!");
                this->_callbacks.at(msg_id)(msg_id, SongState::ERROR);
                this->_callbacks.erase(msg_id);
                return; //callback only notified in success case
            }

            auto verifier = flatbuffers::Verifier(reinterpret_cast<uint8_t*>(buf.response), buf.size);
            bool ok = VerifySongMessageBuffer(verifier);

            if(!ok){
                SPDLOG_WARN("Received a faulty song_like response which does not conform to the song_like_protocol!");
                this->_callbacks.at(msg_id)(msg_id, SongState::ERROR);
                this->_callbacks.erase(msg_id);
                return;
            }

            auto song_msg = GetSongMessage(buf.response);
            if(song_msg->payload_type() != MessagePayload::Response){
                SPDLOG_WARN("Received song_like message with something else than response payload!");
                this->_callbacks.at(msg_id)(msg_id, SongState::ERROR);
                this->_callbacks.erase(msg_id);
                return;
            }

            auto response = song_msg->payload_as_Response();
            auto song_state = map_from_fbb_response_kind(response->kind());

            this->_callbacks.at(song_msg->id())(song_msg->id(), song_state);
            this->_callbacks.erase(song_msg->id());
        }, buffer, size);
    }

    static SongState map_from_fbb_response_kind(ResponseKind kind) {
        switch (kind) {
            case ResponseKind::NOT_FOUND: return SongState::NOT_FAVED;
            case ResponseKind::FOUND_FAVED:
            case ResponseKind::FOUND_NOW_FAVED: return SongState::FAVED;
            case ResponseKind::FOUND_NOT_FAVED:
            case ResponseKind::FOUND_NOW_NOT_FAVED: return SongState::NOT_FAVED;
            default: throw std::runtime_error{"Tried to map song message ResponseKind from an unknown state in SongFaver!"};
        }
    }

    static SourceKind map_to_fbb_source_kind(SongSourceKind kind) {
        switch (kind) {
            case SongSourceKind::RADIO: return SourceKind::RADIO;
            case SongSourceKind::BLUETOOTH: return SourceKind::BLUETOOTH;
            case SongSourceKind::SNAPCAST: return SourceKind::SNAPCAST;
            default: throw std::runtime_error{"Failed to map Source Kind in SongFaver!"};
        }
    }

    uint64_t new_msg_id() {
        if(!_conf.enabled)
            return -1;
        return _last_msg_id++;
    }

    uint64_t _last_msg_id;
    std::unordered_map<uint64_t, std::function<void(uint64_t, SongState)>> _callbacks;
    Configuration::SongFaverConfig _conf;
    std::unique_ptr<NetworkController> &_net;
};

SongFaver::SongFaver(std::unique_ptr<NetworkController> &net, Configuration::SongFaverConfig conf)
        : _impl{std::make_unique<Impl>(net, std::move(conf))} {}

SongFaver::~SongFaver() = default;

uint64_t SongFaver::new_msg_id() {
    return _impl->new_msg_id();
}

void SongFaver::get_state(uint64_t msg_id, SongSource const &source, Song const &s, std::function<void(uint64_t, SongState)> cb) {
    _impl->send_request(msg_id, RequestAction::INFO, source, s, std::move(cb));
}

void SongFaver::fav_song(uint64_t msg_id, SongSource const &source, Song const &s,
                         std::function<void(uint64_t, SongState)> cb) {
    _impl->send_request(msg_id, RequestAction::FAV, source, s, std::move(cb));
}

void SongFaver::unfav_song(uint64_t msg_id, SongSource const &source, Song const &s,
                           std::function<void(uint64_t, SongState)> cb) {
    _impl->send_request(msg_id, RequestAction::UNFAV, source, s, std::move(cb));
}