// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: server_gateway.proto
// Protobuf C++ Version: 4.25.1

#ifndef GOOGLE_PROTOBUF_INCLUDED_server_5fgateway_2eproto_2epb_2eh
#define GOOGLE_PROTOBUF_INCLUDED_server_5fgateway_2eproto_2epb_2eh

#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "google/protobuf/port_def.inc"
#if PROTOBUF_VERSION < 4025000
#error "This file was generated by a newer version of protoc which is"
#error "incompatible with your Protocol Buffer headers. Please update"
#error "your headers."
#endif  // PROTOBUF_VERSION

#if 4025001 < PROTOBUF_MIN_PROTOC_VERSION
#error "This file was generated by an older version of protoc which is"
#error "incompatible with your Protocol Buffer headers. Please"
#error "regenerate this file with a newer version of protoc."
#endif  // PROTOBUF_MIN_PROTOC_VERSION
#include "google/protobuf/port_undef.inc"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/arena.h"
#include "google/protobuf/arenastring.h"
#include "google/protobuf/generated_message_tctable_decl.h"
#include "google/protobuf/generated_message_util.h"
#include "google/protobuf/metadata_lite.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/message.h"
#include "google/protobuf/repeated_field.h"  // IWYU pragma: export
#include "google/protobuf/extension_set.h"  // IWYU pragma: export
#include "google/protobuf/generated_enum_reflection.h"
#include "google/protobuf/unknown_field_set.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"

#define PROTOBUF_INTERNAL_EXPORT_server_5fgateway_2eproto

namespace google {
namespace protobuf {
namespace internal {
class AnyMetadata;
}  // namespace internal
}  // namespace protobuf
}  // namespace google

// Internal implementation detail -- do not use these members.
struct TableStruct_server_5fgateway_2eproto {
  static const ::uint32_t offsets[];
};
extern const ::google::protobuf::internal::DescriptorTable
    descriptor_table_server_5fgateway_2eproto;
namespace myproject {
class ForwardRequest;
struct ForwardRequestDefaultTypeInternal;
extern ForwardRequestDefaultTypeInternal _ForwardRequest_default_instance_;
class ForwardResponse;
struct ForwardResponseDefaultTypeInternal;
extern ForwardResponseDefaultTypeInternal _ForwardResponse_default_instance_;
}  // namespace myproject
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

namespace myproject {
enum ServiceType : int {
  REQ_LOGIN = 0,
  RES_LOGIN = 1,
  ServiceType_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  ServiceType_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool ServiceType_IsValid(int value);
extern const uint32_t ServiceType_internal_data_[];
constexpr ServiceType ServiceType_MIN = static_cast<ServiceType>(0);
constexpr ServiceType ServiceType_MAX = static_cast<ServiceType>(1);
constexpr int ServiceType_ARRAYSIZE = 1 + 1;
const ::google::protobuf::EnumDescriptor*
ServiceType_descriptor();
template <typename T>
const std::string& ServiceType_Name(T value) {
  static_assert(std::is_same<T, ServiceType>::value ||
                    std::is_integral<T>::value,
                "Incorrect type passed to ServiceType_Name().");
  return ServiceType_Name(static_cast<ServiceType>(value));
}
template <>
inline const std::string& ServiceType_Name(ServiceType value) {
  return ::google::protobuf::internal::NameOfDenseEnum<ServiceType_descriptor,
                                                 0, 1>(
      static_cast<int>(value));
}
inline bool ServiceType_Parse(absl::string_view name, ServiceType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ServiceType>(
      ServiceType_descriptor(), name, value);
}

// ===================================================================


// -------------------------------------------------------------------

class ForwardResponse final :
    public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:myproject.ForwardResponse) */ {
 public:
  inline ForwardResponse() : ForwardResponse(nullptr) {}
  ~ForwardResponse() override;
  template<typename = void>
  explicit PROTOBUF_CONSTEXPR ForwardResponse(::google::protobuf::internal::ConstantInitialized);

  inline ForwardResponse(const ForwardResponse& from)
      : ForwardResponse(nullptr, from) {}
  ForwardResponse(ForwardResponse&& from) noexcept
    : ForwardResponse() {
    *this = ::std::move(from);
  }

  inline ForwardResponse& operator=(const ForwardResponse& from) {
    CopyFrom(from);
    return *this;
  }
  inline ForwardResponse& operator=(ForwardResponse&& from) noexcept {
    if (this == &from) return *this;
    if (GetArena() == from.GetArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance);
  }
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields()
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.mutable_unknown_fields<::google::protobuf::UnknownFieldSet>();
  }

  static const ::google::protobuf::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::google::protobuf::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::google::protobuf::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const ForwardResponse& default_instance() {
    return *internal_default_instance();
  }
  static inline const ForwardResponse* internal_default_instance() {
    return reinterpret_cast<const ForwardResponse*>(
               &_ForwardResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(ForwardResponse& a, ForwardResponse& b) {
    a.Swap(&b);
  }
  inline void Swap(ForwardResponse* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() != nullptr &&
        GetArena() == other->GetArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() == other->GetArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::google::protobuf::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(ForwardResponse* other) {
    if (other == this) return;
    ABSL_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  ForwardResponse* New(::google::protobuf::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<ForwardResponse>(arena);
  }
  using ::google::protobuf::Message::CopyFrom;
  void CopyFrom(const ForwardResponse& from);
  using ::google::protobuf::Message::MergeFrom;
  void MergeFrom( const ForwardResponse& from) {
    ForwardResponse::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::google::protobuf::Message& to_msg, const ::google::protobuf::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  ::size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::google::protobuf::internal::ParseContext* ctx) final;
  ::uint8_t* _InternalSerialize(
      ::uint8_t* target, ::google::protobuf::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const { return _impl_._cached_size_.Get(); }

  private:
  ::google::protobuf::internal::CachedSize* AccessCachedSize() const final;
  void SharedCtor(::google::protobuf::Arena* arena);
  void SharedDtor();
  void InternalSwap(ForwardResponse* other);

  private:
  friend class ::google::protobuf::internal::AnyMetadata;
  static ::absl::string_view FullMessageName() {
    return "myproject.ForwardResponse";
  }
  protected:
  explicit ForwardResponse(::google::protobuf::Arena* arena);
  ForwardResponse(::google::protobuf::Arena* arena, const ForwardResponse& from);
  public:

  static const ClassData _class_data_;
  const ::google::protobuf::Message::ClassData*GetClassData() const final;

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kResponseFieldNumber = 2,
    kSuccessFieldNumber = 1,
  };
  // bytes response = 2;
  void clear_response() ;
  const std::string& response() const;
  template <typename Arg_ = const std::string&, typename... Args_>
  void set_response(Arg_&& arg, Args_... args);
  std::string* mutable_response();
  PROTOBUF_NODISCARD std::string* release_response();
  void set_allocated_response(std::string* value);

  private:
  const std::string& _internal_response() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_response(
      const std::string& value);
  std::string* _internal_mutable_response();

  public:
  // bool success = 1;
  void clear_success() ;
  bool success() const;
  void set_success(bool value);

  private:
  bool _internal_success() const;
  void _internal_set_success(bool value);

  public:
  // @@protoc_insertion_point(class_scope:myproject.ForwardResponse)
 private:
  class _Internal;

  friend class ::google::protobuf::internal::TcParser;
  static const ::google::protobuf::internal::TcParseTable<
      1, 2, 0,
      0, 2>
      _table_;
  friend class ::google::protobuf::MessageLite;
  friend class ::google::protobuf::Arena;
  template <typename T>
  friend class ::google::protobuf::Arena::InternalHelper;
  using InternalArenaConstructable_ = void;
  using DestructorSkippable_ = void;
  struct Impl_ {

        inline explicit constexpr Impl_(
            ::google::protobuf::internal::ConstantInitialized) noexcept;
        inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                              ::google::protobuf::Arena* arena);
        inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                              ::google::protobuf::Arena* arena, const Impl_& from);
    ::google::protobuf::internal::ArenaStringPtr response_;
    bool success_;
    mutable ::google::protobuf::internal::CachedSize _cached_size_;
    PROTOBUF_TSAN_DECLARE_MEMBER
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_server_5fgateway_2eproto;
};// -------------------------------------------------------------------

class ForwardRequest final :
    public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:myproject.ForwardRequest) */ {
 public:
  inline ForwardRequest() : ForwardRequest(nullptr) {}
  ~ForwardRequest() override;
  template<typename = void>
  explicit PROTOBUF_CONSTEXPR ForwardRequest(::google::protobuf::internal::ConstantInitialized);

  inline ForwardRequest(const ForwardRequest& from)
      : ForwardRequest(nullptr, from) {}
  ForwardRequest(ForwardRequest&& from) noexcept
    : ForwardRequest() {
    *this = ::std::move(from);
  }

  inline ForwardRequest& operator=(const ForwardRequest& from) {
    CopyFrom(from);
    return *this;
  }
  inline ForwardRequest& operator=(ForwardRequest&& from) noexcept {
    if (this == &from) return *this;
    if (GetArena() == from.GetArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance);
  }
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields()
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.mutable_unknown_fields<::google::protobuf::UnknownFieldSet>();
  }

  static const ::google::protobuf::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::google::protobuf::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::google::protobuf::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const ForwardRequest& default_instance() {
    return *internal_default_instance();
  }
  static inline const ForwardRequest* internal_default_instance() {
    return reinterpret_cast<const ForwardRequest*>(
               &_ForwardRequest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(ForwardRequest& a, ForwardRequest& b) {
    a.Swap(&b);
  }
  inline void Swap(ForwardRequest* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() != nullptr &&
        GetArena() == other->GetArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() == other->GetArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::google::protobuf::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(ForwardRequest* other) {
    if (other == this) return;
    ABSL_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  ForwardRequest* New(::google::protobuf::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<ForwardRequest>(arena);
  }
  using ::google::protobuf::Message::CopyFrom;
  void CopyFrom(const ForwardRequest& from);
  using ::google::protobuf::Message::MergeFrom;
  void MergeFrom( const ForwardRequest& from) {
    ForwardRequest::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::google::protobuf::Message& to_msg, const ::google::protobuf::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  ::size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::google::protobuf::internal::ParseContext* ctx) final;
  ::uint8_t* _InternalSerialize(
      ::uint8_t* target, ::google::protobuf::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const { return _impl_._cached_size_.Get(); }

  private:
  ::google::protobuf::internal::CachedSize* AccessCachedSize() const final;
  void SharedCtor(::google::protobuf::Arena* arena);
  void SharedDtor();
  void InternalSwap(ForwardRequest* other);

  private:
  friend class ::google::protobuf::internal::AnyMetadata;
  static ::absl::string_view FullMessageName() {
    return "myproject.ForwardRequest";
  }
  protected:
  explicit ForwardRequest(::google::protobuf::Arena* arena);
  ForwardRequest(::google::protobuf::Arena* arena, const ForwardRequest& from);
  public:

  static const ClassData _class_data_;
  const ::google::protobuf::Message::ClassData*GetClassData() const final;

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kPayloadFieldNumber = 2,
    kServiceTypeFieldNumber = 1,
  };
  // bytes payload = 2;
  void clear_payload() ;
  const std::string& payload() const;
  template <typename Arg_ = const std::string&, typename... Args_>
  void set_payload(Arg_&& arg, Args_... args);
  std::string* mutable_payload();
  PROTOBUF_NODISCARD std::string* release_payload();
  void set_allocated_payload(std::string* value);

  private:
  const std::string& _internal_payload() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_payload(
      const std::string& value);
  std::string* _internal_mutable_payload();

  public:
  // .myproject.ServiceType service_type = 1;
  void clear_service_type() ;
  ::myproject::ServiceType service_type() const;
  void set_service_type(::myproject::ServiceType value);

  private:
  ::myproject::ServiceType _internal_service_type() const;
  void _internal_set_service_type(::myproject::ServiceType value);

  public:
  // @@protoc_insertion_point(class_scope:myproject.ForwardRequest)
 private:
  class _Internal;

  friend class ::google::protobuf::internal::TcParser;
  static const ::google::protobuf::internal::TcParseTable<
      1, 2, 0,
      0, 2>
      _table_;
  friend class ::google::protobuf::MessageLite;
  friend class ::google::protobuf::Arena;
  template <typename T>
  friend class ::google::protobuf::Arena::InternalHelper;
  using InternalArenaConstructable_ = void;
  using DestructorSkippable_ = void;
  struct Impl_ {

        inline explicit constexpr Impl_(
            ::google::protobuf::internal::ConstantInitialized) noexcept;
        inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                              ::google::protobuf::Arena* arena);
        inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                              ::google::protobuf::Arena* arena, const Impl_& from);
    ::google::protobuf::internal::ArenaStringPtr payload_;
    int service_type_;
    mutable ::google::protobuf::internal::CachedSize _cached_size_;
    PROTOBUF_TSAN_DECLARE_MEMBER
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_server_5fgateway_2eproto;
};

// ===================================================================




// ===================================================================


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// -------------------------------------------------------------------

// ForwardRequest

// .myproject.ServiceType service_type = 1;
inline void ForwardRequest::clear_service_type() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  _impl_.service_type_ = 0;
}
inline ::myproject::ServiceType ForwardRequest::service_type() const {
  // @@protoc_insertion_point(field_get:myproject.ForwardRequest.service_type)
  return _internal_service_type();
}
inline void ForwardRequest::set_service_type(::myproject::ServiceType value) {
  _internal_set_service_type(value);
  // @@protoc_insertion_point(field_set:myproject.ForwardRequest.service_type)
}
inline ::myproject::ServiceType ForwardRequest::_internal_service_type() const {
  PROTOBUF_TSAN_READ(&_impl_._tsan_detect_race);
  return static_cast<::myproject::ServiceType>(_impl_.service_type_);
}
inline void ForwardRequest::_internal_set_service_type(::myproject::ServiceType value) {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ;
  _impl_.service_type_ = value;
}

// bytes payload = 2;
inline void ForwardRequest::clear_payload() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  _impl_.payload_.ClearToEmpty();
}
inline const std::string& ForwardRequest::payload() const
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  // @@protoc_insertion_point(field_get:myproject.ForwardRequest.payload)
  return _internal_payload();
}
template <typename Arg_, typename... Args_>
inline PROTOBUF_ALWAYS_INLINE void ForwardRequest::set_payload(Arg_&& arg,
                                                     Args_... args) {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ;
  _impl_.payload_.SetBytes(static_cast<Arg_&&>(arg), args..., GetArena());
  // @@protoc_insertion_point(field_set:myproject.ForwardRequest.payload)
}
inline std::string* ForwardRequest::mutable_payload() ABSL_ATTRIBUTE_LIFETIME_BOUND {
  std::string* _s = _internal_mutable_payload();
  // @@protoc_insertion_point(field_mutable:myproject.ForwardRequest.payload)
  return _s;
}
inline const std::string& ForwardRequest::_internal_payload() const {
  PROTOBUF_TSAN_READ(&_impl_._tsan_detect_race);
  return _impl_.payload_.Get();
}
inline void ForwardRequest::_internal_set_payload(const std::string& value) {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ;
  _impl_.payload_.Set(value, GetArena());
}
inline std::string* ForwardRequest::_internal_mutable_payload() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ;
  return _impl_.payload_.Mutable( GetArena());
}
inline std::string* ForwardRequest::release_payload() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  // @@protoc_insertion_point(field_release:myproject.ForwardRequest.payload)
  return _impl_.payload_.Release();
}
inline void ForwardRequest::set_allocated_payload(std::string* value) {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  _impl_.payload_.SetAllocated(value, GetArena());
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
        if (_impl_.payload_.IsDefault()) {
          _impl_.payload_.Set("", GetArena());
        }
  #endif  // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:myproject.ForwardRequest.payload)
}

// -------------------------------------------------------------------

// ForwardResponse

// bool success = 1;
inline void ForwardResponse::clear_success() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  _impl_.success_ = false;
}
inline bool ForwardResponse::success() const {
  // @@protoc_insertion_point(field_get:myproject.ForwardResponse.success)
  return _internal_success();
}
inline void ForwardResponse::set_success(bool value) {
  _internal_set_success(value);
  // @@protoc_insertion_point(field_set:myproject.ForwardResponse.success)
}
inline bool ForwardResponse::_internal_success() const {
  PROTOBUF_TSAN_READ(&_impl_._tsan_detect_race);
  return _impl_.success_;
}
inline void ForwardResponse::_internal_set_success(bool value) {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ;
  _impl_.success_ = value;
}

// bytes response = 2;
inline void ForwardResponse::clear_response() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  _impl_.response_.ClearToEmpty();
}
inline const std::string& ForwardResponse::response() const
    ABSL_ATTRIBUTE_LIFETIME_BOUND {
  // @@protoc_insertion_point(field_get:myproject.ForwardResponse.response)
  return _internal_response();
}
template <typename Arg_, typename... Args_>
inline PROTOBUF_ALWAYS_INLINE void ForwardResponse::set_response(Arg_&& arg,
                                                     Args_... args) {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ;
  _impl_.response_.SetBytes(static_cast<Arg_&&>(arg), args..., GetArena());
  // @@protoc_insertion_point(field_set:myproject.ForwardResponse.response)
}
inline std::string* ForwardResponse::mutable_response() ABSL_ATTRIBUTE_LIFETIME_BOUND {
  std::string* _s = _internal_mutable_response();
  // @@protoc_insertion_point(field_mutable:myproject.ForwardResponse.response)
  return _s;
}
inline const std::string& ForwardResponse::_internal_response() const {
  PROTOBUF_TSAN_READ(&_impl_._tsan_detect_race);
  return _impl_.response_.Get();
}
inline void ForwardResponse::_internal_set_response(const std::string& value) {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ;
  _impl_.response_.Set(value, GetArena());
}
inline std::string* ForwardResponse::_internal_mutable_response() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ;
  return _impl_.response_.Mutable( GetArena());
}
inline std::string* ForwardResponse::release_response() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  // @@protoc_insertion_point(field_release:myproject.ForwardResponse.response)
  return _impl_.response_.Release();
}
inline void ForwardResponse::set_allocated_response(std::string* value) {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  _impl_.response_.SetAllocated(value, GetArena());
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
        if (_impl_.response_.IsDefault()) {
          _impl_.response_.Set("", GetArena());
        }
  #endif  // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:myproject.ForwardResponse.response)
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)
}  // namespace myproject


namespace google {
namespace protobuf {

template <>
struct is_proto_enum<::myproject::ServiceType> : std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor<::myproject::ServiceType>() {
  return ::myproject::ServiceType_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_INCLUDED_server_5fgateway_2eproto_2epb_2eh
