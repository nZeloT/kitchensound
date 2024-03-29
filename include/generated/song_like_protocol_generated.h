// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SONGLIKEPROTOCOL_H_
#define FLATBUFFERS_GENERATED_SONGLIKEPROTOCOL_H_

#include "flatbuffers/flatbuffers.h"

struct SongMessage;

struct Response;

struct SongInfo;

struct SongSourceInfo;

struct Request;

enum class MessagePayload : uint8_t {
  NONE = 0,
  Response = 1,
  Request = 2,
  MIN = NONE,
  MAX = Request
};

inline const MessagePayload (&EnumValuesMessagePayload())[3] {
  static const MessagePayload values[] = {
    MessagePayload::NONE,
    MessagePayload::Response,
    MessagePayload::Request
  };
  return values;
}

inline const char * const *EnumNamesMessagePayload() {
  static const char * const names[] = {
    "NONE",
    "Response",
    "Request",
    nullptr
  };
  return names;
}

inline const char *EnumNameMessagePayload(MessagePayload e) {
  if (e < MessagePayload::NONE || e > MessagePayload::Request) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesMessagePayload()[index];
}

template<typename T> struct MessagePayloadTraits {
  static const MessagePayload enum_value = MessagePayload::NONE;
};

template<> struct MessagePayloadTraits<Response> {
  static const MessagePayload enum_value = MessagePayload::Response;
};

template<> struct MessagePayloadTraits<Request> {
  static const MessagePayload enum_value = MessagePayload::Request;
};

bool VerifyMessagePayload(flatbuffers::Verifier &verifier, const void *obj, MessagePayload type);
bool VerifyMessagePayloadVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types);

enum class ResponseKind : uint8_t {
  NOT_FOUND = 0,
  FOUND_NOT_FAVED = 1,
  FOUND_FAVED = 2,
  FOUND_NOW_FAVED = 3,
  FOUND_NOW_NOT_FAVED = 4,
  MIN = NOT_FOUND,
  MAX = FOUND_NOW_NOT_FAVED
};

inline const ResponseKind (&EnumValuesResponseKind())[5] {
  static const ResponseKind values[] = {
    ResponseKind::NOT_FOUND,
    ResponseKind::FOUND_NOT_FAVED,
    ResponseKind::FOUND_FAVED,
    ResponseKind::FOUND_NOW_FAVED,
    ResponseKind::FOUND_NOW_NOT_FAVED
  };
  return values;
}

inline const char * const *EnumNamesResponseKind() {
  static const char * const names[] = {
    "NOT_FOUND",
    "FOUND_NOT_FAVED",
    "FOUND_FAVED",
    "FOUND_NOW_FAVED",
    "FOUND_NOW_NOT_FAVED",
    nullptr
  };
  return names;
}

inline const char *EnumNameResponseKind(ResponseKind e) {
  if (e < ResponseKind::NOT_FOUND || e > ResponseKind::FOUND_NOW_NOT_FAVED) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesResponseKind()[index];
}

enum class SourceKind : uint8_t {
  RADIO = 0,
  BLUETOOTH = 1,
  SNAPCAST = 2,
  MIN = RADIO,
  MAX = SNAPCAST
};

inline const SourceKind (&EnumValuesSourceKind())[3] {
  static const SourceKind values[] = {
    SourceKind::RADIO,
    SourceKind::BLUETOOTH,
    SourceKind::SNAPCAST
  };
  return values;
}

inline const char * const *EnumNamesSourceKind() {
  static const char * const names[] = {
    "RADIO",
    "BLUETOOTH",
    "SNAPCAST",
    nullptr
  };
  return names;
}

inline const char *EnumNameSourceKind(SourceKind e) {
  if (e < SourceKind::RADIO || e > SourceKind::SNAPCAST) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesSourceKind()[index];
}

enum class RequestAction : uint8_t {
  INFO = 0,
  FAV = 1,
  UNFAV = 2,
  MIN = INFO,
  MAX = UNFAV
};

inline const RequestAction (&EnumValuesRequestAction())[3] {
  static const RequestAction values[] = {
    RequestAction::INFO,
    RequestAction::FAV,
    RequestAction::UNFAV
  };
  return values;
}

inline const char * const *EnumNamesRequestAction() {
  static const char * const names[] = {
    "INFO",
    "FAV",
    "UNFAV",
    nullptr
  };
  return names;
}

inline const char *EnumNameRequestAction(RequestAction e) {
  if (e < RequestAction::INFO || e > RequestAction::UNFAV) return "";
  const size_t index = static_cast<size_t>(e);
  return EnumNamesRequestAction()[index];
}

struct SongMessage FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_ID = 4,
    VT_PAYLOAD_TYPE = 6,
    VT_PAYLOAD = 8
  };
  uint64_t id() const {
    return GetField<uint64_t>(VT_ID, 0);
  }
  MessagePayload payload_type() const {
    return static_cast<MessagePayload>(GetField<uint8_t>(VT_PAYLOAD_TYPE, 0));
  }
  const void *payload() const {
    return GetPointer<const void *>(VT_PAYLOAD);
  }
  template<typename T> const T *payload_as() const;
  const Response *payload_as_Response() const {
    return payload_type() == MessagePayload::Response ? static_cast<const Response *>(payload()) : nullptr;
  }
  const Request *payload_as_Request() const {
    return payload_type() == MessagePayload::Request ? static_cast<const Request *>(payload()) : nullptr;
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_ID) &&
           VerifyField<uint8_t>(verifier, VT_PAYLOAD_TYPE) &&
           VerifyOffsetRequired(verifier, VT_PAYLOAD) &&
           VerifyMessagePayload(verifier, payload(), payload_type()) &&
           verifier.EndTable();
  }
};

template<> inline const Response *SongMessage::payload_as<Response>() const {
  return payload_as_Response();
}

template<> inline const Request *SongMessage::payload_as<Request>() const {
  return payload_as_Request();
}

struct SongMessageBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_id(uint64_t id) {
    fbb_.AddElement<uint64_t>(SongMessage::VT_ID, id, 0);
  }
  void add_payload_type(MessagePayload payload_type) {
    fbb_.AddElement<uint8_t>(SongMessage::VT_PAYLOAD_TYPE, static_cast<uint8_t>(payload_type), 0);
  }
  void add_payload(flatbuffers::Offset<void> payload) {
    fbb_.AddOffset(SongMessage::VT_PAYLOAD, payload);
  }
  explicit SongMessageBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  SongMessageBuilder &operator=(const SongMessageBuilder &);
  flatbuffers::Offset<SongMessage> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SongMessage>(end);
    fbb_.Required(o, SongMessage::VT_PAYLOAD);
    return o;
  }
};

inline flatbuffers::Offset<SongMessage> CreateSongMessage(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t id = 0,
    MessagePayload payload_type = MessagePayload::NONE,
    flatbuffers::Offset<void> payload = 0) {
  SongMessageBuilder builder_(_fbb);
  builder_.add_id(id);
  builder_.add_payload(payload);
  builder_.add_payload_type(payload_type);
  return builder_.Finish();
}

struct Response FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_KIND = 4
  };
  ResponseKind kind() const {
    return static_cast<ResponseKind>(GetField<uint8_t>(VT_KIND, 0));
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_KIND) &&
           verifier.EndTable();
  }
};

struct ResponseBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_kind(ResponseKind kind) {
    fbb_.AddElement<uint8_t>(Response::VT_KIND, static_cast<uint8_t>(kind), 0);
  }
  explicit ResponseBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ResponseBuilder &operator=(const ResponseBuilder &);
  flatbuffers::Offset<Response> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Response>(end);
    return o;
  }
};

inline flatbuffers::Offset<Response> CreateResponse(
    flatbuffers::FlatBufferBuilder &_fbb,
    ResponseKind kind = ResponseKind::NOT_FOUND) {
  ResponseBuilder builder_(_fbb);
  builder_.add_kind(kind);
  return builder_.Finish();
}

struct SongInfo FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_RAW_META = 4,
    VT_SONG_TITLE = 6,
    VT_SONG_ARTIST = 8,
    VT_SONG_ALBUM = 10
  };
  const flatbuffers::String *raw_meta() const {
    return GetPointer<const flatbuffers::String *>(VT_RAW_META);
  }
  const flatbuffers::String *song_title() const {
    return GetPointer<const flatbuffers::String *>(VT_SONG_TITLE);
  }
  const flatbuffers::String *song_artist() const {
    return GetPointer<const flatbuffers::String *>(VT_SONG_ARTIST);
  }
  const flatbuffers::String *song_album() const {
    return GetPointer<const flatbuffers::String *>(VT_SONG_ALBUM);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_RAW_META) &&
           verifier.VerifyString(raw_meta()) &&
           VerifyOffset(verifier, VT_SONG_TITLE) &&
           verifier.VerifyString(song_title()) &&
           VerifyOffset(verifier, VT_SONG_ARTIST) &&
           verifier.VerifyString(song_artist()) &&
           VerifyOffset(verifier, VT_SONG_ALBUM) &&
           verifier.VerifyString(song_album()) &&
           verifier.EndTable();
  }
};

struct SongInfoBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_raw_meta(flatbuffers::Offset<flatbuffers::String> raw_meta) {
    fbb_.AddOffset(SongInfo::VT_RAW_META, raw_meta);
  }
  void add_song_title(flatbuffers::Offset<flatbuffers::String> song_title) {
    fbb_.AddOffset(SongInfo::VT_SONG_TITLE, song_title);
  }
  void add_song_artist(flatbuffers::Offset<flatbuffers::String> song_artist) {
    fbb_.AddOffset(SongInfo::VT_SONG_ARTIST, song_artist);
  }
  void add_song_album(flatbuffers::Offset<flatbuffers::String> song_album) {
    fbb_.AddOffset(SongInfo::VT_SONG_ALBUM, song_album);
  }
  explicit SongInfoBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  SongInfoBuilder &operator=(const SongInfoBuilder &);
  flatbuffers::Offset<SongInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SongInfo>(end);
    fbb_.Required(o, SongInfo::VT_RAW_META);
    return o;
  }
};

inline flatbuffers::Offset<SongInfo> CreateSongInfo(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> raw_meta = 0,
    flatbuffers::Offset<flatbuffers::String> song_title = 0,
    flatbuffers::Offset<flatbuffers::String> song_artist = 0,
    flatbuffers::Offset<flatbuffers::String> song_album = 0) {
  SongInfoBuilder builder_(_fbb);
  builder_.add_song_album(song_album);
  builder_.add_song_artist(song_artist);
  builder_.add_song_title(song_title);
  builder_.add_raw_meta(raw_meta);
  return builder_.Finish();
}

inline flatbuffers::Offset<SongInfo> CreateSongInfoDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *raw_meta = nullptr,
    const char *song_title = nullptr,
    const char *song_artist = nullptr,
    const char *song_album = nullptr) {
  auto raw_meta__ = raw_meta ? _fbb.CreateString(raw_meta) : 0;
  auto song_title__ = song_title ? _fbb.CreateString(song_title) : 0;
  auto song_artist__ = song_artist ? _fbb.CreateString(song_artist) : 0;
  auto song_album__ = song_album ? _fbb.CreateString(song_album) : 0;
  return CreateSongInfo(
      _fbb,
      raw_meta__,
      song_title__,
      song_artist__,
      song_album__);
}

struct SongSourceInfo FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SOURCE_KIND = 4,
    VT_SOURCE_NAME = 6
  };
  SourceKind source_kind() const {
    return static_cast<SourceKind>(GetField<uint8_t>(VT_SOURCE_KIND, 0));
  }
  const flatbuffers::String *source_name() const {
    return GetPointer<const flatbuffers::String *>(VT_SOURCE_NAME);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint8_t>(verifier, VT_SOURCE_KIND) &&
           VerifyOffsetRequired(verifier, VT_SOURCE_NAME) &&
           verifier.VerifyString(source_name()) &&
           verifier.EndTable();
  }
};

struct SongSourceInfoBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_source_kind(SourceKind source_kind) {
    fbb_.AddElement<uint8_t>(SongSourceInfo::VT_SOURCE_KIND, static_cast<uint8_t>(source_kind), 0);
  }
  void add_source_name(flatbuffers::Offset<flatbuffers::String> source_name) {
    fbb_.AddOffset(SongSourceInfo::VT_SOURCE_NAME, source_name);
  }
  explicit SongSourceInfoBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  SongSourceInfoBuilder &operator=(const SongSourceInfoBuilder &);
  flatbuffers::Offset<SongSourceInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SongSourceInfo>(end);
    fbb_.Required(o, SongSourceInfo::VT_SOURCE_NAME);
    return o;
  }
};

inline flatbuffers::Offset<SongSourceInfo> CreateSongSourceInfo(
    flatbuffers::FlatBufferBuilder &_fbb,
    SourceKind source_kind = SourceKind::RADIO,
    flatbuffers::Offset<flatbuffers::String> source_name = 0) {
  SongSourceInfoBuilder builder_(_fbb);
  builder_.add_source_name(source_name);
  builder_.add_source_kind(source_kind);
  return builder_.Finish();
}

inline flatbuffers::Offset<SongSourceInfo> CreateSongSourceInfoDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    SourceKind source_kind = SourceKind::RADIO,
    const char *source_name = nullptr) {
  auto source_name__ = source_name ? _fbb.CreateString(source_name) : 0;
  return CreateSongSourceInfo(
      _fbb,
      source_kind,
      source_name__);
}

struct Request FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_REQUESTING_PARTY = 4,
    VT_ACTION_KIND = 6,
    VT_SONG_SOURCE_INFO = 8,
    VT_SONG_INFO = 10
  };
  const flatbuffers::String *requesting_party() const {
    return GetPointer<const flatbuffers::String *>(VT_REQUESTING_PARTY);
  }
  RequestAction action_kind() const {
    return static_cast<RequestAction>(GetField<uint8_t>(VT_ACTION_KIND, 0));
  }
  const SongSourceInfo *song_source_info() const {
    return GetPointer<const SongSourceInfo *>(VT_SONG_SOURCE_INFO);
  }
  const SongInfo *song_info() const {
    return GetPointer<const SongInfo *>(VT_SONG_INFO);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_REQUESTING_PARTY) &&
           verifier.VerifyString(requesting_party()) &&
           VerifyField<uint8_t>(verifier, VT_ACTION_KIND) &&
           VerifyOffsetRequired(verifier, VT_SONG_SOURCE_INFO) &&
           verifier.VerifyTable(song_source_info()) &&
           VerifyOffsetRequired(verifier, VT_SONG_INFO) &&
           verifier.VerifyTable(song_info()) &&
           verifier.EndTable();
  }
};

struct RequestBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_requesting_party(flatbuffers::Offset<flatbuffers::String> requesting_party) {
    fbb_.AddOffset(Request::VT_REQUESTING_PARTY, requesting_party);
  }
  void add_action_kind(RequestAction action_kind) {
    fbb_.AddElement<uint8_t>(Request::VT_ACTION_KIND, static_cast<uint8_t>(action_kind), 0);
  }
  void add_song_source_info(flatbuffers::Offset<SongSourceInfo> song_source_info) {
    fbb_.AddOffset(Request::VT_SONG_SOURCE_INFO, song_source_info);
  }
  void add_song_info(flatbuffers::Offset<SongInfo> song_info) {
    fbb_.AddOffset(Request::VT_SONG_INFO, song_info);
  }
  explicit RequestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RequestBuilder &operator=(const RequestBuilder &);
  flatbuffers::Offset<Request> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<Request>(end);
    fbb_.Required(o, Request::VT_REQUESTING_PARTY);
    fbb_.Required(o, Request::VT_SONG_SOURCE_INFO);
    fbb_.Required(o, Request::VT_SONG_INFO);
    return o;
  }
};

inline flatbuffers::Offset<Request> CreateRequest(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> requesting_party = 0,
    RequestAction action_kind = RequestAction::INFO,
    flatbuffers::Offset<SongSourceInfo> song_source_info = 0,
    flatbuffers::Offset<SongInfo> song_info = 0) {
  RequestBuilder builder_(_fbb);
  builder_.add_song_info(song_info);
  builder_.add_song_source_info(song_source_info);
  builder_.add_requesting_party(requesting_party);
  builder_.add_action_kind(action_kind);
  return builder_.Finish();
}

inline flatbuffers::Offset<Request> CreateRequestDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *requesting_party = nullptr,
    RequestAction action_kind = RequestAction::INFO,
    flatbuffers::Offset<SongSourceInfo> song_source_info = 0,
    flatbuffers::Offset<SongInfo> song_info = 0) {
  auto requesting_party__ = requesting_party ? _fbb.CreateString(requesting_party) : 0;
  return CreateRequest(
      _fbb,
      requesting_party__,
      action_kind,
      song_source_info,
      song_info);
}

inline bool VerifyMessagePayload(flatbuffers::Verifier &verifier, const void *obj, MessagePayload type) {
  switch (type) {
    case MessagePayload::NONE: {
      return true;
    }
    case MessagePayload::Response: {
      auto ptr = reinterpret_cast<const Response *>(obj);
      return verifier.VerifyTable(ptr);
    }
    case MessagePayload::Request: {
      auto ptr = reinterpret_cast<const Request *>(obj);
      return verifier.VerifyTable(ptr);
    }
    default: return false;
  }
}

inline bool VerifyMessagePayloadVector(flatbuffers::Verifier &verifier, const flatbuffers::Vector<flatbuffers::Offset<void>> *values, const flatbuffers::Vector<uint8_t> *types) {
  if (!values || !types) return !values && !types;
  if (values->size() != types->size()) return false;
  for (flatbuffers::uoffset_t i = 0; i < values->size(); ++i) {
    if (!VerifyMessagePayload(
        verifier,  values->Get(i), types->GetEnum<MessagePayload>(i))) {
      return false;
    }
  }
  return true;
}

inline const SongMessage *GetSongMessage(const void *buf) {
  return flatbuffers::GetRoot<SongMessage>(buf);
}

inline const SongMessage *GetSizePrefixedSongMessage(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<SongMessage>(buf);
}

inline bool VerifySongMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<SongMessage>(nullptr);
}

inline bool VerifySizePrefixedSongMessageBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<SongMessage>(nullptr);
}

inline void FinishSongMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<SongMessage> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedSongMessageBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<SongMessage> root) {
  fbb.FinishSizePrefixed(root);
}

#endif  // FLATBUFFERS_GENERATED_SONGLIKEPROTOCOL_H_
