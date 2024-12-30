// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: server_gateway.proto

#include "server_gateway.pb.h"

#include <algorithm>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/extension_set.h"
#include "google/protobuf/wire_format_lite.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/reflection_ops.h"
#include "google/protobuf/wire_format.h"
#include "google/protobuf/generated_message_tctable_impl.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"
PROTOBUF_PRAGMA_INIT_SEG
namespace _pb = ::google::protobuf;
namespace _pbi = ::google::protobuf::internal;
namespace _fl = ::google::protobuf::internal::field_layout;
namespace rpc_server {

inline constexpr ForwardResponse::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : response_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        success_{false},
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR ForwardResponse::ForwardResponse(::_pbi::ConstantInitialized)
    : _impl_(::_pbi::ConstantInitialized()) {}
struct ForwardResponseDefaultTypeInternal {
  PROTOBUF_CONSTEXPR ForwardResponseDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~ForwardResponseDefaultTypeInternal() {}
  union {
    ForwardResponse _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 ForwardResponseDefaultTypeInternal _ForwardResponse_default_instance_;

inline constexpr ForwardRequest::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : payload_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        service_type_{static_cast< ::rpc_server::ServiceType >(0)},
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR ForwardRequest::ForwardRequest(::_pbi::ConstantInitialized)
    : _impl_(::_pbi::ConstantInitialized()) {}
struct ForwardRequestDefaultTypeInternal {
  PROTOBUF_CONSTEXPR ForwardRequestDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~ForwardRequestDefaultTypeInternal() {}
  union {
    ForwardRequest _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 ForwardRequestDefaultTypeInternal _ForwardRequest_default_instance_;
}  // namespace rpc_server
static ::_pb::Metadata file_level_metadata_server_5fgateway_2eproto[2];
static const ::_pb::EnumDescriptor* file_level_enum_descriptors_server_5fgateway_2eproto[1];
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_server_5fgateway_2eproto = nullptr;
const ::uint32_t TableStruct_server_5fgateway_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(
    protodesc_cold) = {
    ~0u,  // no _has_bits_
    PROTOBUF_FIELD_OFFSET(::rpc_server::ForwardRequest, _internal_metadata_),
    ~0u,  // no _extensions_
    ~0u,  // no _oneof_case_
    ~0u,  // no _weak_field_map_
    ~0u,  // no _inlined_string_donated_
    ~0u,  // no _split_
    ~0u,  // no sizeof(Split)
    PROTOBUF_FIELD_OFFSET(::rpc_server::ForwardRequest, _impl_.service_type_),
    PROTOBUF_FIELD_OFFSET(::rpc_server::ForwardRequest, _impl_.payload_),
    ~0u,  // no _has_bits_
    PROTOBUF_FIELD_OFFSET(::rpc_server::ForwardResponse, _internal_metadata_),
    ~0u,  // no _extensions_
    ~0u,  // no _oneof_case_
    ~0u,  // no _weak_field_map_
    ~0u,  // no _inlined_string_donated_
    ~0u,  // no _split_
    ~0u,  // no sizeof(Split)
    PROTOBUF_FIELD_OFFSET(::rpc_server::ForwardResponse, _impl_.success_),
    PROTOBUF_FIELD_OFFSET(::rpc_server::ForwardResponse, _impl_.response_),
};

static const ::_pbi::MigrationSchema
    schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
        {0, -1, -1, sizeof(::rpc_server::ForwardRequest)},
        {10, -1, -1, sizeof(::rpc_server::ForwardResponse)},
};

static const ::_pb::Message* const file_default_instances[] = {
    &::rpc_server::_ForwardRequest_default_instance_._instance,
    &::rpc_server::_ForwardResponse_default_instance_._instance,
};
const char descriptor_table_protodef_server_5fgateway_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
    "\n\024server_gateway.proto\022\nrpc_server\"P\n\016Fo"
    "rwardRequest\022-\n\014service_type\030\001 \001(\0162\027.rpc"
    "_server.ServiceType\022\017\n\007payload\030\002 \001(\014\"4\n\017"
    "ForwardResponse\022\017\n\007success\030\001 \001(\010\022\020\n\010resp"
    "onse\030\002 \001(\014*+\n\013ServiceType\022\r\n\tREQ_LOGIN\020\000"
    "\022\r\n\tRES_LOGIN\020\0012Z\n\rGatewayServer\022I\n\016Requ"
    "estForward\022\032.rpc_server.ForwardRequest\032\033"
    ".rpc_server.ForwardResponseb\006proto3"
};
static ::absl::once_flag descriptor_table_server_5fgateway_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_server_5fgateway_2eproto = {
    false,
    false,
    315,
    descriptor_table_protodef_server_5fgateway_2eproto,
    "server_gateway.proto",
    &descriptor_table_server_5fgateway_2eproto_once,
    nullptr,
    0,
    2,
    schemas,
    file_default_instances,
    TableStruct_server_5fgateway_2eproto::offsets,
    file_level_metadata_server_5fgateway_2eproto,
    file_level_enum_descriptors_server_5fgateway_2eproto,
    file_level_service_descriptors_server_5fgateway_2eproto,
};

// This function exists to be marked as weak.
// It can significantly speed up compilation by breaking up LLVM's SCC
// in the .pb.cc translation units. Large translation units see a
// reduction of more than 35% of walltime for optimized builds. Without
// the weak attribute all the messages in the file, including all the
// vtables and everything they use become part of the same SCC through
// a cycle like:
// GetMetadata -> descriptor table -> default instances ->
//   vtables -> GetMetadata
// By adding a weak function here we break the connection from the
// individual vtables back into the descriptor table.
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_server_5fgateway_2eproto_getter() {
  return &descriptor_table_server_5fgateway_2eproto;
}
// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2
static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_server_5fgateway_2eproto(&descriptor_table_server_5fgateway_2eproto);
namespace rpc_server {
const ::google::protobuf::EnumDescriptor* ServiceType_descriptor() {
  ::google::protobuf::internal::AssignDescriptors(&descriptor_table_server_5fgateway_2eproto);
  return file_level_enum_descriptors_server_5fgateway_2eproto[0];
}
PROTOBUF_CONSTINIT const uint32_t ServiceType_internal_data_[] = {
    131072u, 0u, };
bool ServiceType_IsValid(int value) {
  return 0 <= value && value <= 1;
}
// ===================================================================

class ForwardRequest::_Internal {
 public:
};

ForwardRequest::ForwardRequest(::google::protobuf::Arena* arena)
    : ::google::protobuf::Message(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:rpc_server.ForwardRequest)
}
inline PROTOBUF_NDEBUG_INLINE ForwardRequest::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from)
      : payload_(arena, from.payload_),
        _cached_size_{0} {}

ForwardRequest::ForwardRequest(
    ::google::protobuf::Arena* arena,
    const ForwardRequest& from)
    : ::google::protobuf::Message(arena) {
  ForwardRequest* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_);
  _impl_.service_type_ = from._impl_.service_type_;

  // @@protoc_insertion_point(copy_constructor:rpc_server.ForwardRequest)
}
inline PROTOBUF_NDEBUG_INLINE ForwardRequest::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : payload_(arena),
        _cached_size_{0} {}

inline void ForwardRequest::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
  _impl_.service_type_ = {};
}
ForwardRequest::~ForwardRequest() {
  // @@protoc_insertion_point(destructor:rpc_server.ForwardRequest)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void ForwardRequest::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.payload_.Destroy();
  _impl_.~Impl_();
}

PROTOBUF_NOINLINE void ForwardRequest::Clear() {
// @@protoc_insertion_point(message_clear_start:rpc_server.ForwardRequest)
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.payload_.ClearToEmpty();
  _impl_.service_type_ = 0;
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

const char* ForwardRequest::_InternalParse(
    const char* ptr, ::_pbi::ParseContext* ctx) {
  ptr = ::_pbi::TcParser::ParseLoop(this, ptr, ctx, &_table_.header);
  return ptr;
}


PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<1, 2, 0, 0, 2> ForwardRequest::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    2, 8,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967292,  // skipmap
    offsetof(decltype(_table_), field_entries),
    2,  // num_field_entries
    0,  // num_aux_entries
    offsetof(decltype(_table_), field_names),  // no aux_entries
    &_ForwardRequest_default_instance_._instance,
    ::_pbi::TcParser::GenericFallback,  // fallback
  }, {{
    // bytes payload = 2;
    {::_pbi::TcParser::FastBS1,
     {18, 63, 0, PROTOBUF_FIELD_OFFSET(ForwardRequest, _impl_.payload_)}},
    // .rpc_server.ServiceType service_type = 1;
    {::_pbi::TcParser::SingularVarintNoZag1<::uint32_t, offsetof(ForwardRequest, _impl_.service_type_), 63>(),
     {8, 63, 0, PROTOBUF_FIELD_OFFSET(ForwardRequest, _impl_.service_type_)}},
  }}, {{
    65535, 65535
  }}, {{
    // .rpc_server.ServiceType service_type = 1;
    {PROTOBUF_FIELD_OFFSET(ForwardRequest, _impl_.service_type_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kOpenEnum)},
    // bytes payload = 2;
    {PROTOBUF_FIELD_OFFSET(ForwardRequest, _impl_.payload_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kBytes | ::_fl::kRepAString)},
  }},
  // no aux_entries
  {{
  }},
};

::uint8_t* ForwardRequest::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:rpc_server.ForwardRequest)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // .rpc_server.ServiceType service_type = 1;
  if (this->_internal_service_type() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteEnumToArray(
        1, this->_internal_service_type(), target);
  }

  // bytes payload = 2;
  if (!this->_internal_payload().empty()) {
    const std::string& _s = this->_internal_payload();
    target = stream->WriteBytesMaybeAliased(2, _s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target =
        ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:rpc_server.ForwardRequest)
  return target;
}

::size_t ForwardRequest::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:rpc_server.ForwardRequest)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // bytes payload = 2;
  if (!this->_internal_payload().empty()) {
    total_size += 1 + ::google::protobuf::internal::WireFormatLite::BytesSize(
                                    this->_internal_payload());
  }

  // .rpc_server.ServiceType service_type = 1;
  if (this->_internal_service_type() != 0) {
    total_size += 1 +
                  ::_pbi::WireFormatLite::EnumSize(this->_internal_service_type());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::google::protobuf::Message::ClassData ForwardRequest::_class_data_ = {
    ForwardRequest::MergeImpl,
    nullptr,  // OnDemandRegisterArenaDtor
};
const ::google::protobuf::Message::ClassData* ForwardRequest::GetClassData() const {
  return &_class_data_;
}

void ForwardRequest::MergeImpl(::google::protobuf::Message& to_msg, const ::google::protobuf::Message& from_msg) {
  auto* const _this = static_cast<ForwardRequest*>(&to_msg);
  auto& from = static_cast<const ForwardRequest&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:rpc_server.ForwardRequest)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_payload().empty()) {
    _this->_internal_set_payload(from._internal_payload());
  }
  if (from._internal_service_type() != 0) {
    _this->_internal_set_service_type(from._internal_service_type());
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void ForwardRequest::CopyFrom(const ForwardRequest& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:rpc_server.ForwardRequest)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

PROTOBUF_NOINLINE bool ForwardRequest::IsInitialized() const {
  return true;
}

::_pbi::CachedSize* ForwardRequest::AccessCachedSize() const {
  return &_impl_._cached_size_;
}
void ForwardRequest::InternalSwap(ForwardRequest* PROTOBUF_RESTRICT other) {
  using std::swap;
  auto* arena = GetArena();
  ABSL_DCHECK_EQ(arena, other->GetArena());
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.payload_, &other->_impl_.payload_, arena);
  swap(_impl_.service_type_, other->_impl_.service_type_);
}

::google::protobuf::Metadata ForwardRequest::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_server_5fgateway_2eproto_getter, &descriptor_table_server_5fgateway_2eproto_once,
      file_level_metadata_server_5fgateway_2eproto[0]);
}
// ===================================================================

class ForwardResponse::_Internal {
 public:
};

ForwardResponse::ForwardResponse(::google::protobuf::Arena* arena)
    : ::google::protobuf::Message(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:rpc_server.ForwardResponse)
}
inline PROTOBUF_NDEBUG_INLINE ForwardResponse::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from)
      : response_(arena, from.response_),
        _cached_size_{0} {}

ForwardResponse::ForwardResponse(
    ::google::protobuf::Arena* arena,
    const ForwardResponse& from)
    : ::google::protobuf::Message(arena) {
  ForwardResponse* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_);
  _impl_.success_ = from._impl_.success_;

  // @@protoc_insertion_point(copy_constructor:rpc_server.ForwardResponse)
}
inline PROTOBUF_NDEBUG_INLINE ForwardResponse::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : response_(arena),
        _cached_size_{0} {}

inline void ForwardResponse::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
  _impl_.success_ = {};
}
ForwardResponse::~ForwardResponse() {
  // @@protoc_insertion_point(destructor:rpc_server.ForwardResponse)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void ForwardResponse::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.response_.Destroy();
  _impl_.~Impl_();
}

PROTOBUF_NOINLINE void ForwardResponse::Clear() {
// @@protoc_insertion_point(message_clear_start:rpc_server.ForwardResponse)
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.response_.ClearToEmpty();
  _impl_.success_ = false;
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

const char* ForwardResponse::_InternalParse(
    const char* ptr, ::_pbi::ParseContext* ctx) {
  ptr = ::_pbi::TcParser::ParseLoop(this, ptr, ctx, &_table_.header);
  return ptr;
}


PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<1, 2, 0, 0, 2> ForwardResponse::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    2, 8,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967292,  // skipmap
    offsetof(decltype(_table_), field_entries),
    2,  // num_field_entries
    0,  // num_aux_entries
    offsetof(decltype(_table_), field_names),  // no aux_entries
    &_ForwardResponse_default_instance_._instance,
    ::_pbi::TcParser::GenericFallback,  // fallback
  }, {{
    // bytes response = 2;
    {::_pbi::TcParser::FastBS1,
     {18, 63, 0, PROTOBUF_FIELD_OFFSET(ForwardResponse, _impl_.response_)}},
    // bool success = 1;
    {::_pbi::TcParser::SingularVarintNoZag1<bool, offsetof(ForwardResponse, _impl_.success_), 63>(),
     {8, 63, 0, PROTOBUF_FIELD_OFFSET(ForwardResponse, _impl_.success_)}},
  }}, {{
    65535, 65535
  }}, {{
    // bool success = 1;
    {PROTOBUF_FIELD_OFFSET(ForwardResponse, _impl_.success_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kBool)},
    // bytes response = 2;
    {PROTOBUF_FIELD_OFFSET(ForwardResponse, _impl_.response_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kBytes | ::_fl::kRepAString)},
  }},
  // no aux_entries
  {{
  }},
};

::uint8_t* ForwardResponse::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:rpc_server.ForwardResponse)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // bool success = 1;
  if (this->_internal_success() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteBoolToArray(
        1, this->_internal_success(), target);
  }

  // bytes response = 2;
  if (!this->_internal_response().empty()) {
    const std::string& _s = this->_internal_response();
    target = stream->WriteBytesMaybeAliased(2, _s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target =
        ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:rpc_server.ForwardResponse)
  return target;
}

::size_t ForwardResponse::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:rpc_server.ForwardResponse)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // bytes response = 2;
  if (!this->_internal_response().empty()) {
    total_size += 1 + ::google::protobuf::internal::WireFormatLite::BytesSize(
                                    this->_internal_response());
  }

  // bool success = 1;
  if (this->_internal_success() != 0) {
    total_size += 2;
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::google::protobuf::Message::ClassData ForwardResponse::_class_data_ = {
    ForwardResponse::MergeImpl,
    nullptr,  // OnDemandRegisterArenaDtor
};
const ::google::protobuf::Message::ClassData* ForwardResponse::GetClassData() const {
  return &_class_data_;
}

void ForwardResponse::MergeImpl(::google::protobuf::Message& to_msg, const ::google::protobuf::Message& from_msg) {
  auto* const _this = static_cast<ForwardResponse*>(&to_msg);
  auto& from = static_cast<const ForwardResponse&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:rpc_server.ForwardResponse)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_response().empty()) {
    _this->_internal_set_response(from._internal_response());
  }
  if (from._internal_success() != 0) {
    _this->_internal_set_success(from._internal_success());
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void ForwardResponse::CopyFrom(const ForwardResponse& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:rpc_server.ForwardResponse)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

PROTOBUF_NOINLINE bool ForwardResponse::IsInitialized() const {
  return true;
}

::_pbi::CachedSize* ForwardResponse::AccessCachedSize() const {
  return &_impl_._cached_size_;
}
void ForwardResponse::InternalSwap(ForwardResponse* PROTOBUF_RESTRICT other) {
  using std::swap;
  auto* arena = GetArena();
  ABSL_DCHECK_EQ(arena, other->GetArena());
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.response_, &other->_impl_.response_, arena);
        swap(_impl_.success_, other->_impl_.success_);
}

::google::protobuf::Metadata ForwardResponse::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_server_5fgateway_2eproto_getter, &descriptor_table_server_5fgateway_2eproto_once,
      file_level_metadata_server_5fgateway_2eproto[1]);
}
// @@protoc_insertion_point(namespace_scope)
}  // namespace rpc_server
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
#include "google/protobuf/port_undef.inc"
