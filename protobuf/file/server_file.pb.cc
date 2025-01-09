// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: server_file.proto

#include "server_file.pb.h"

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

inline constexpr UploadRes::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : message_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        success_{false},
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR UploadRes::UploadRes(::_pbi::ConstantInitialized)
    : _impl_(::_pbi::ConstantInitialized()) {}
struct UploadResDefaultTypeInternal {
  PROTOBUF_CONSTEXPR UploadResDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~UploadResDefaultTypeInternal() {}
  union {
    UploadRes _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 UploadResDefaultTypeInternal _UploadRes_default_instance_;

inline constexpr UploadReq::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : account_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        file_name_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR UploadReq::UploadReq(::_pbi::ConstantInitialized)
    : _impl_(::_pbi::ConstantInitialized()) {}
struct UploadReqDefaultTypeInternal {
  PROTOBUF_CONSTEXPR UploadReqDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~UploadReqDefaultTypeInternal() {}
  union {
    UploadReq _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 UploadReqDefaultTypeInternal _UploadReq_default_instance_;
}  // namespace rpc_server
static ::_pb::Metadata file_level_metadata_server_5ffile_2eproto[2];
static constexpr const ::_pb::EnumDescriptor**
    file_level_enum_descriptors_server_5ffile_2eproto = nullptr;
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_server_5ffile_2eproto = nullptr;
const ::uint32_t TableStruct_server_5ffile_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(
    protodesc_cold) = {
    ~0u,  // no _has_bits_
    PROTOBUF_FIELD_OFFSET(::rpc_server::UploadReq, _internal_metadata_),
    ~0u,  // no _extensions_
    ~0u,  // no _oneof_case_
    ~0u,  // no _weak_field_map_
    ~0u,  // no _inlined_string_donated_
    ~0u,  // no _split_
    ~0u,  // no sizeof(Split)
    PROTOBUF_FIELD_OFFSET(::rpc_server::UploadReq, _impl_.account_),
    PROTOBUF_FIELD_OFFSET(::rpc_server::UploadReq, _impl_.file_name_),
    ~0u,  // no _has_bits_
    PROTOBUF_FIELD_OFFSET(::rpc_server::UploadRes, _internal_metadata_),
    ~0u,  // no _extensions_
    ~0u,  // no _oneof_case_
    ~0u,  // no _weak_field_map_
    ~0u,  // no _inlined_string_donated_
    ~0u,  // no _split_
    ~0u,  // no sizeof(Split)
    PROTOBUF_FIELD_OFFSET(::rpc_server::UploadRes, _impl_.success_),
    PROTOBUF_FIELD_OFFSET(::rpc_server::UploadRes, _impl_.message_),
};

static const ::_pbi::MigrationSchema
    schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
        {0, -1, -1, sizeof(::rpc_server::UploadReq)},
        {10, -1, -1, sizeof(::rpc_server::UploadRes)},
};

static const ::_pb::Message* const file_default_instances[] = {
    &::rpc_server::_UploadReq_default_instance_._instance,
    &::rpc_server::_UploadRes_default_instance_._instance,
};
const char descriptor_table_protodef_server_5ffile_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
    "\n\021server_file.proto\022\nrpc_server\"/\n\tUploa"
    "dReq\022\017\n\007account\030\001 \001(\014\022\021\n\tfile_name\030\002 \001(\014"
    "\"-\n\tUploadRes\022\017\n\007success\030\001 \001(\010\022\017\n\007messag"
    "e\030\002 \001(\0142D\n\nFileServer\0226\n\006Upload\022\025.rpc_se"
    "rver.UploadReq\032\025.rpc_server.UploadResb\006p"
    "roto3"
};
static ::absl::once_flag descriptor_table_server_5ffile_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_server_5ffile_2eproto = {
    false,
    false,
    205,
    descriptor_table_protodef_server_5ffile_2eproto,
    "server_file.proto",
    &descriptor_table_server_5ffile_2eproto_once,
    nullptr,
    0,
    2,
    schemas,
    file_default_instances,
    TableStruct_server_5ffile_2eproto::offsets,
    file_level_metadata_server_5ffile_2eproto,
    file_level_enum_descriptors_server_5ffile_2eproto,
    file_level_service_descriptors_server_5ffile_2eproto,
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
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_server_5ffile_2eproto_getter() {
  return &descriptor_table_server_5ffile_2eproto;
}
// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2
static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_server_5ffile_2eproto(&descriptor_table_server_5ffile_2eproto);
namespace rpc_server {
// ===================================================================

class UploadReq::_Internal {
 public:
};

UploadReq::UploadReq(::google::protobuf::Arena* arena)
    : ::google::protobuf::Message(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:rpc_server.UploadReq)
}
inline PROTOBUF_NDEBUG_INLINE UploadReq::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from)
      : account_(arena, from.account_),
        file_name_(arena, from.file_name_),
        _cached_size_{0} {}

UploadReq::UploadReq(
    ::google::protobuf::Arena* arena,
    const UploadReq& from)
    : ::google::protobuf::Message(arena) {
  UploadReq* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_);

  // @@protoc_insertion_point(copy_constructor:rpc_server.UploadReq)
}
inline PROTOBUF_NDEBUG_INLINE UploadReq::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : account_(arena),
        file_name_(arena),
        _cached_size_{0} {}

inline void UploadReq::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
}
UploadReq::~UploadReq() {
  // @@protoc_insertion_point(destructor:rpc_server.UploadReq)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void UploadReq::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.account_.Destroy();
  _impl_.file_name_.Destroy();
  _impl_.~Impl_();
}

PROTOBUF_NOINLINE void UploadReq::Clear() {
// @@protoc_insertion_point(message_clear_start:rpc_server.UploadReq)
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.account_.ClearToEmpty();
  _impl_.file_name_.ClearToEmpty();
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

const char* UploadReq::_InternalParse(
    const char* ptr, ::_pbi::ParseContext* ctx) {
  ptr = ::_pbi::TcParser::ParseLoop(this, ptr, ctx, &_table_.header);
  return ptr;
}


PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<1, 2, 0, 0, 2> UploadReq::_table_ = {
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
    &_UploadReq_default_instance_._instance,
    ::_pbi::TcParser::GenericFallback,  // fallback
  }, {{
    // bytes file_name = 2;
    {::_pbi::TcParser::FastBS1,
     {18, 63, 0, PROTOBUF_FIELD_OFFSET(UploadReq, _impl_.file_name_)}},
    // bytes account = 1;
    {::_pbi::TcParser::FastBS1,
     {10, 63, 0, PROTOBUF_FIELD_OFFSET(UploadReq, _impl_.account_)}},
  }}, {{
    65535, 65535
  }}, {{
    // bytes account = 1;
    {PROTOBUF_FIELD_OFFSET(UploadReq, _impl_.account_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kBytes | ::_fl::kRepAString)},
    // bytes file_name = 2;
    {PROTOBUF_FIELD_OFFSET(UploadReq, _impl_.file_name_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kBytes | ::_fl::kRepAString)},
  }},
  // no aux_entries
  {{
  }},
};

::uint8_t* UploadReq::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:rpc_server.UploadReq)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // bytes account = 1;
  if (!this->_internal_account().empty()) {
    const std::string& _s = this->_internal_account();
    target = stream->WriteBytesMaybeAliased(1, _s, target);
  }

  // bytes file_name = 2;
  if (!this->_internal_file_name().empty()) {
    const std::string& _s = this->_internal_file_name();
    target = stream->WriteBytesMaybeAliased(2, _s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target =
        ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:rpc_server.UploadReq)
  return target;
}

::size_t UploadReq::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:rpc_server.UploadReq)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // bytes account = 1;
  if (!this->_internal_account().empty()) {
    total_size += 1 + ::google::protobuf::internal::WireFormatLite::BytesSize(
                                    this->_internal_account());
  }

  // bytes file_name = 2;
  if (!this->_internal_file_name().empty()) {
    total_size += 1 + ::google::protobuf::internal::WireFormatLite::BytesSize(
                                    this->_internal_file_name());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::google::protobuf::Message::ClassData UploadReq::_class_data_ = {
    UploadReq::MergeImpl,
    nullptr,  // OnDemandRegisterArenaDtor
};
const ::google::protobuf::Message::ClassData* UploadReq::GetClassData() const {
  return &_class_data_;
}

void UploadReq::MergeImpl(::google::protobuf::Message& to_msg, const ::google::protobuf::Message& from_msg) {
  auto* const _this = static_cast<UploadReq*>(&to_msg);
  auto& from = static_cast<const UploadReq&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:rpc_server.UploadReq)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_account().empty()) {
    _this->_internal_set_account(from._internal_account());
  }
  if (!from._internal_file_name().empty()) {
    _this->_internal_set_file_name(from._internal_file_name());
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void UploadReq::CopyFrom(const UploadReq& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:rpc_server.UploadReq)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

PROTOBUF_NOINLINE bool UploadReq::IsInitialized() const {
  return true;
}

::_pbi::CachedSize* UploadReq::AccessCachedSize() const {
  return &_impl_._cached_size_;
}
void UploadReq::InternalSwap(UploadReq* PROTOBUF_RESTRICT other) {
  using std::swap;
  auto* arena = GetArena();
  ABSL_DCHECK_EQ(arena, other->GetArena());
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.account_, &other->_impl_.account_, arena);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.file_name_, &other->_impl_.file_name_, arena);
}

::google::protobuf::Metadata UploadReq::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_server_5ffile_2eproto_getter, &descriptor_table_server_5ffile_2eproto_once,
      file_level_metadata_server_5ffile_2eproto[0]);
}
// ===================================================================

class UploadRes::_Internal {
 public:
};

UploadRes::UploadRes(::google::protobuf::Arena* arena)
    : ::google::protobuf::Message(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:rpc_server.UploadRes)
}
inline PROTOBUF_NDEBUG_INLINE UploadRes::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from)
      : message_(arena, from.message_),
        _cached_size_{0} {}

UploadRes::UploadRes(
    ::google::protobuf::Arena* arena,
    const UploadRes& from)
    : ::google::protobuf::Message(arena) {
  UploadRes* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_);
  _impl_.success_ = from._impl_.success_;

  // @@protoc_insertion_point(copy_constructor:rpc_server.UploadRes)
}
inline PROTOBUF_NDEBUG_INLINE UploadRes::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : message_(arena),
        _cached_size_{0} {}

inline void UploadRes::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
  _impl_.success_ = {};
}
UploadRes::~UploadRes() {
  // @@protoc_insertion_point(destructor:rpc_server.UploadRes)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void UploadRes::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.message_.Destroy();
  _impl_.~Impl_();
}

PROTOBUF_NOINLINE void UploadRes::Clear() {
// @@protoc_insertion_point(message_clear_start:rpc_server.UploadRes)
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.message_.ClearToEmpty();
  _impl_.success_ = false;
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

const char* UploadRes::_InternalParse(
    const char* ptr, ::_pbi::ParseContext* ctx) {
  ptr = ::_pbi::TcParser::ParseLoop(this, ptr, ctx, &_table_.header);
  return ptr;
}


PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<1, 2, 0, 0, 2> UploadRes::_table_ = {
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
    &_UploadRes_default_instance_._instance,
    ::_pbi::TcParser::GenericFallback,  // fallback
  }, {{
    // bytes message = 2;
    {::_pbi::TcParser::FastBS1,
     {18, 63, 0, PROTOBUF_FIELD_OFFSET(UploadRes, _impl_.message_)}},
    // bool success = 1;
    {::_pbi::TcParser::SingularVarintNoZag1<bool, offsetof(UploadRes, _impl_.success_), 63>(),
     {8, 63, 0, PROTOBUF_FIELD_OFFSET(UploadRes, _impl_.success_)}},
  }}, {{
    65535, 65535
  }}, {{
    // bool success = 1;
    {PROTOBUF_FIELD_OFFSET(UploadRes, _impl_.success_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kBool)},
    // bytes message = 2;
    {PROTOBUF_FIELD_OFFSET(UploadRes, _impl_.message_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kBytes | ::_fl::kRepAString)},
  }},
  // no aux_entries
  {{
  }},
};

::uint8_t* UploadRes::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:rpc_server.UploadRes)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // bool success = 1;
  if (this->_internal_success() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteBoolToArray(
        1, this->_internal_success(), target);
  }

  // bytes message = 2;
  if (!this->_internal_message().empty()) {
    const std::string& _s = this->_internal_message();
    target = stream->WriteBytesMaybeAliased(2, _s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target =
        ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:rpc_server.UploadRes)
  return target;
}

::size_t UploadRes::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:rpc_server.UploadRes)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // bytes message = 2;
  if (!this->_internal_message().empty()) {
    total_size += 1 + ::google::protobuf::internal::WireFormatLite::BytesSize(
                                    this->_internal_message());
  }

  // bool success = 1;
  if (this->_internal_success() != 0) {
    total_size += 2;
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::google::protobuf::Message::ClassData UploadRes::_class_data_ = {
    UploadRes::MergeImpl,
    nullptr,  // OnDemandRegisterArenaDtor
};
const ::google::protobuf::Message::ClassData* UploadRes::GetClassData() const {
  return &_class_data_;
}

void UploadRes::MergeImpl(::google::protobuf::Message& to_msg, const ::google::protobuf::Message& from_msg) {
  auto* const _this = static_cast<UploadRes*>(&to_msg);
  auto& from = static_cast<const UploadRes&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:rpc_server.UploadRes)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_message().empty()) {
    _this->_internal_set_message(from._internal_message());
  }
  if (from._internal_success() != 0) {
    _this->_internal_set_success(from._internal_success());
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void UploadRes::CopyFrom(const UploadRes& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:rpc_server.UploadRes)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

PROTOBUF_NOINLINE bool UploadRes::IsInitialized() const {
  return true;
}

::_pbi::CachedSize* UploadRes::AccessCachedSize() const {
  return &_impl_._cached_size_;
}
void UploadRes::InternalSwap(UploadRes* PROTOBUF_RESTRICT other) {
  using std::swap;
  auto* arena = GetArena();
  ABSL_DCHECK_EQ(arena, other->GetArena());
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.message_, &other->_impl_.message_, arena);
        swap(_impl_.success_, other->_impl_.success_);
}

::google::protobuf::Metadata UploadRes::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_server_5ffile_2eproto_getter, &descriptor_table_server_5ffile_2eproto_once,
      file_level_metadata_server_5ffile_2eproto[1]);
}
// @@protoc_insertion_point(namespace_scope)
}  // namespace rpc_server
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
#include "google/protobuf/port_undef.inc"
