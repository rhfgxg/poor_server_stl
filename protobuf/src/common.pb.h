// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: common.proto
// Protobuf C++ Version: 4.25.1

#ifndef GOOGLE_PROTOBUF_INCLUDED_common_2eproto_2epb_2eh
#define GOOGLE_PROTOBUF_INCLUDED_common_2eproto_2epb_2eh

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
#include "google/protobuf/repeated_field.h"  // IWYU pragma: export
#include "google/protobuf/extension_set.h"  // IWYU pragma: export
#include "google/protobuf/generated_enum_reflection.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"

#define PROTOBUF_INTERNAL_EXPORT_common_2eproto

namespace google {
namespace protobuf {
namespace internal {
class AnyMetadata;
}  // namespace internal
}  // namespace protobuf
}  // namespace google

// Internal implementation detail -- do not use these members.
struct TableStruct_common_2eproto {
  static const ::uint32_t offsets[];
};
extern const ::google::protobuf::internal::DescriptorTable
    descriptor_table_common_2eproto;
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

namespace rpc_server {
enum ServerType : int {
  UNKNOWN = 0,
  BATTLE = 1,
  CENTRAL = 2,
  CHAT = 3,
  DB = 4,
  FILE = 5,
  GATEWAY = 6,
  LOGIC = 7,
  LOGIN = 8,
  MATCHING = 9,
  STORE = 10,
  ServerType_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  ServerType_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool ServerType_IsValid(int value);
extern const uint32_t ServerType_internal_data_[];
constexpr ServerType ServerType_MIN = static_cast<ServerType>(0);
constexpr ServerType ServerType_MAX = static_cast<ServerType>(10);
constexpr int ServerType_ARRAYSIZE = 10 + 1;
const ::google::protobuf::EnumDescriptor*
ServerType_descriptor();
template <typename T>
const std::string& ServerType_Name(T value) {
  static_assert(std::is_same<T, ServerType>::value ||
                    std::is_integral<T>::value,
                "Incorrect type passed to ServerType_Name().");
  return ServerType_Name(static_cast<ServerType>(value));
}
template <>
inline const std::string& ServerType_Name(ServerType value) {
  return ::google::protobuf::internal::NameOfDenseEnum<ServerType_descriptor,
                                                 0, 10>(
      static_cast<int>(value));
}
inline bool ServerType_Parse(absl::string_view name, ServerType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ServerType>(
      ServerType_descriptor(), name, value);
}
enum ServiceType : int {
  REQ_REGISTER = 0,
  RES_REGISTER = 1,
  REQ_LOGIN = 2,
  RES_LOGIN = 3,
  REQ_LOGOUT = 4,
  RES_LOGOUT = 5,
  REQ_CHANGE_PASSWORD = 6,
  RES_CHANGE_PASSWORD = 7,
  REQ_HEARTBEAT = 8,
  RES_HEARTBEAT = 9,
  REQ_FILE_TRANSMISSION_READY = 10,
  RES_FILE_TRANSMISSION_READY = 11,
  REQ_FILE_UPLOAD = 12,
  RES_FILE_UPLOAD = 13,
  REQ_FILE_DOWNLOAD = 14,
  RES_FILE_DOWNLOAD = 15,
  REQ_FILE_DELETE = 16,
  RES_FILE_DELETE = 17,
  REQ_FILE_LIST = 18,
  RES_FILE_LIST = 19,
  ServiceType_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  ServiceType_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool ServiceType_IsValid(int value);
extern const uint32_t ServiceType_internal_data_[];
constexpr ServiceType ServiceType_MIN = static_cast<ServiceType>(0);
constexpr ServiceType ServiceType_MAX = static_cast<ServiceType>(19);
constexpr int ServiceType_ARRAYSIZE = 19 + 1;
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
                                                 0, 19>(
      static_cast<int>(value));
}
inline bool ServiceType_Parse(absl::string_view name, ServiceType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ServiceType>(
      ServiceType_descriptor(), name, value);
}

// ===================================================================



// ===================================================================




// ===================================================================


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)
}  // namespace rpc_server


namespace google {
namespace protobuf {

template <>
struct is_proto_enum<::rpc_server::ServerType> : std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor<::rpc_server::ServerType>() {
  return ::rpc_server::ServerType_descriptor();
}
template <>
struct is_proto_enum<::rpc_server::ServiceType> : std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor<::rpc_server::ServiceType>() {
  return ::rpc_server::ServiceType_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_INCLUDED_common_2eproto_2epb_2eh
