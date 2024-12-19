// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: server_central.proto
#ifndef GRPC_server_5fcentral_2eproto__INCLUDED
#define GRPC_server_5fcentral_2eproto__INCLUDED

#include "server_central.pb.h"

#include <functional>
#include <grpcpp/generic/async_generic_service.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/client_context.h>
#include <grpcpp/completion_queue.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/proto_utils.h>
#include <grpcpp/impl/rpc_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/status.h>
#include <grpcpp/support/stub_options.h>
#include <grpcpp/support/sync_stream.h>

namespace myproject {

class CentralServer final {
 public:
  static constexpr char const* service_full_name() {
    return "myproject.CentralServer";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status RegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::myproject::RegisterServerResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::RegisterServerResponse>> AsyncRegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::RegisterServerResponse>>(AsyncRegisterServerRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::RegisterServerResponse>> PrepareAsyncRegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::RegisterServerResponse>>(PrepareAsyncRegisterServerRaw(context, request, cq));
    }
    // 服务器注册
    virtual ::grpc::Status UnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::myproject::UnregisterServerResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::UnregisterServerResponse>> AsyncUnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::UnregisterServerResponse>>(AsyncUnregisterServerRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::UnregisterServerResponse>> PrepareAsyncUnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::UnregisterServerResponse>>(PrepareAsyncUnregisterServerRaw(context, request, cq));
    }
    // 服务器断开
    virtual ::grpc::Status GetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::myproject::ConnectPoorResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ConnectPoorResponse>> AsyncGetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ConnectPoorResponse>>(AsyncGetConnectPoorRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ConnectPoorResponse>> PrepareAsyncGetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ConnectPoorResponse>>(PrepareAsyncGetConnectPoorRaw(context, request, cq));
    }
    // 获取连接池信息
    class async_interface {
     public:
      virtual ~async_interface() {}
      virtual void RegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest* request, ::myproject::RegisterServerResponse* response, std::function<void(::grpc::Status)>) = 0;
      virtual void RegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest* request, ::myproject::RegisterServerResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      // 服务器注册
      virtual void UnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest* request, ::myproject::UnregisterServerResponse* response, std::function<void(::grpc::Status)>) = 0;
      virtual void UnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest* request, ::myproject::UnregisterServerResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      // 服务器断开
      virtual void GetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest* request, ::myproject::ConnectPoorResponse* response, std::function<void(::grpc::Status)>) = 0;
      virtual void GetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest* request, ::myproject::ConnectPoorResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      // 获取连接池信息
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::RegisterServerResponse>* AsyncRegisterServerRaw(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::RegisterServerResponse>* PrepareAsyncRegisterServerRaw(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::UnregisterServerResponse>* AsyncUnregisterServerRaw(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::UnregisterServerResponse>* PrepareAsyncUnregisterServerRaw(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ConnectPoorResponse>* AsyncGetConnectPoorRaw(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ConnectPoorResponse>* PrepareAsyncGetConnectPoorRaw(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    ::grpc::Status RegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::myproject::RegisterServerResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::RegisterServerResponse>> AsyncRegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::RegisterServerResponse>>(AsyncRegisterServerRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::RegisterServerResponse>> PrepareAsyncRegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::RegisterServerResponse>>(PrepareAsyncRegisterServerRaw(context, request, cq));
    }
    ::grpc::Status UnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::myproject::UnregisterServerResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::UnregisterServerResponse>> AsyncUnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::UnregisterServerResponse>>(AsyncUnregisterServerRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::UnregisterServerResponse>> PrepareAsyncUnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::UnregisterServerResponse>>(PrepareAsyncUnregisterServerRaw(context, request, cq));
    }
    ::grpc::Status GetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::myproject::ConnectPoorResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::ConnectPoorResponse>> AsyncGetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::ConnectPoorResponse>>(AsyncGetConnectPoorRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::ConnectPoorResponse>> PrepareAsyncGetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::ConnectPoorResponse>>(PrepareAsyncGetConnectPoorRaw(context, request, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void RegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest* request, ::myproject::RegisterServerResponse* response, std::function<void(::grpc::Status)>) override;
      void RegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest* request, ::myproject::RegisterServerResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
      void UnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest* request, ::myproject::UnregisterServerResponse* response, std::function<void(::grpc::Status)>) override;
      void UnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest* request, ::myproject::UnregisterServerResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
      void GetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest* request, ::myproject::ConnectPoorResponse* response, std::function<void(::grpc::Status)>) override;
      void GetConnectPoor(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest* request, ::myproject::ConnectPoorResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
     private:
      friend class Stub;
      explicit async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class async* async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::myproject::RegisterServerResponse>* AsyncRegisterServerRaw(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::myproject::RegisterServerResponse>* PrepareAsyncRegisterServerRaw(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::myproject::UnregisterServerResponse>* AsyncUnregisterServerRaw(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::myproject::UnregisterServerResponse>* PrepareAsyncUnregisterServerRaw(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::myproject::ConnectPoorResponse>* AsyncGetConnectPoorRaw(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::myproject::ConnectPoorResponse>* PrepareAsyncGetConnectPoorRaw(::grpc::ClientContext* context, const ::myproject::ConnectPoorRequest& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_RegisterServer_;
    const ::grpc::internal::RpcMethod rpcmethod_UnregisterServer_;
    const ::grpc::internal::RpcMethod rpcmethod_GetConnectPoor_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status RegisterServer(::grpc::ServerContext* context, const ::myproject::RegisterServerRequest* request, ::myproject::RegisterServerResponse* response);
    // 服务器注册
    virtual ::grpc::Status UnregisterServer(::grpc::ServerContext* context, const ::myproject::UnregisterServerRequest* request, ::myproject::UnregisterServerResponse* response);
    // 服务器断开
    virtual ::grpc::Status GetConnectPoor(::grpc::ServerContext* context, const ::myproject::ConnectPoorRequest* request, ::myproject::ConnectPoorResponse* response);
    // 获取连接池信息
  };
  template <class BaseClass>
  class WithAsyncMethod_RegisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_RegisterServer() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_RegisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RegisterServer(::grpc::ServerContext* /*context*/, const ::myproject::RegisterServerRequest* /*request*/, ::myproject::RegisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestRegisterServer(::grpc::ServerContext* context, ::myproject::RegisterServerRequest* request, ::grpc::ServerAsyncResponseWriter< ::myproject::RegisterServerResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_UnregisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_UnregisterServer() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_UnregisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status UnregisterServer(::grpc::ServerContext* /*context*/, const ::myproject::UnregisterServerRequest* /*request*/, ::myproject::UnregisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestUnregisterServer(::grpc::ServerContext* context, ::myproject::UnregisterServerRequest* request, ::grpc::ServerAsyncResponseWriter< ::myproject::UnregisterServerResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_GetConnectPoor : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_GetConnectPoor() {
      ::grpc::Service::MarkMethodAsync(2);
    }
    ~WithAsyncMethod_GetConnectPoor() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetConnectPoor(::grpc::ServerContext* /*context*/, const ::myproject::ConnectPoorRequest* /*request*/, ::myproject::ConnectPoorResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetConnectPoor(::grpc::ServerContext* context, ::myproject::ConnectPoorRequest* request, ::grpc::ServerAsyncResponseWriter< ::myproject::ConnectPoorResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(2, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_RegisterServer<WithAsyncMethod_UnregisterServer<WithAsyncMethod_GetConnectPoor<Service > > > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_RegisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_RegisterServer() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::myproject::RegisterServerRequest, ::myproject::RegisterServerResponse>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::myproject::RegisterServerRequest* request, ::myproject::RegisterServerResponse* response) { return this->RegisterServer(context, request, response); }));}
    void SetMessageAllocatorFor_RegisterServer(
        ::grpc::MessageAllocator< ::myproject::RegisterServerRequest, ::myproject::RegisterServerResponse>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::myproject::RegisterServerRequest, ::myproject::RegisterServerResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_RegisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RegisterServer(::grpc::ServerContext* /*context*/, const ::myproject::RegisterServerRequest* /*request*/, ::myproject::RegisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* RegisterServer(
      ::grpc::CallbackServerContext* /*context*/, const ::myproject::RegisterServerRequest* /*request*/, ::myproject::RegisterServerResponse* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_UnregisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_UnregisterServer() {
      ::grpc::Service::MarkMethodCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::myproject::UnregisterServerRequest, ::myproject::UnregisterServerResponse>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::myproject::UnregisterServerRequest* request, ::myproject::UnregisterServerResponse* response) { return this->UnregisterServer(context, request, response); }));}
    void SetMessageAllocatorFor_UnregisterServer(
        ::grpc::MessageAllocator< ::myproject::UnregisterServerRequest, ::myproject::UnregisterServerResponse>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(1);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::myproject::UnregisterServerRequest, ::myproject::UnregisterServerResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_UnregisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status UnregisterServer(::grpc::ServerContext* /*context*/, const ::myproject::UnregisterServerRequest* /*request*/, ::myproject::UnregisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* UnregisterServer(
      ::grpc::CallbackServerContext* /*context*/, const ::myproject::UnregisterServerRequest* /*request*/, ::myproject::UnregisterServerResponse* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_GetConnectPoor : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_GetConnectPoor() {
      ::grpc::Service::MarkMethodCallback(2,
          new ::grpc::internal::CallbackUnaryHandler< ::myproject::ConnectPoorRequest, ::myproject::ConnectPoorResponse>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::myproject::ConnectPoorRequest* request, ::myproject::ConnectPoorResponse* response) { return this->GetConnectPoor(context, request, response); }));}
    void SetMessageAllocatorFor_GetConnectPoor(
        ::grpc::MessageAllocator< ::myproject::ConnectPoorRequest, ::myproject::ConnectPoorResponse>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(2);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::myproject::ConnectPoorRequest, ::myproject::ConnectPoorResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_GetConnectPoor() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetConnectPoor(::grpc::ServerContext* /*context*/, const ::myproject::ConnectPoorRequest* /*request*/, ::myproject::ConnectPoorResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* GetConnectPoor(
      ::grpc::CallbackServerContext* /*context*/, const ::myproject::ConnectPoorRequest* /*request*/, ::myproject::ConnectPoorResponse* /*response*/)  { return nullptr; }
  };
  typedef WithCallbackMethod_RegisterServer<WithCallbackMethod_UnregisterServer<WithCallbackMethod_GetConnectPoor<Service > > > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_RegisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_RegisterServer() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_RegisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RegisterServer(::grpc::ServerContext* /*context*/, const ::myproject::RegisterServerRequest* /*request*/, ::myproject::RegisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_UnregisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_UnregisterServer() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_UnregisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status UnregisterServer(::grpc::ServerContext* /*context*/, const ::myproject::UnregisterServerRequest* /*request*/, ::myproject::UnregisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_GetConnectPoor : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_GetConnectPoor() {
      ::grpc::Service::MarkMethodGeneric(2);
    }
    ~WithGenericMethod_GetConnectPoor() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetConnectPoor(::grpc::ServerContext* /*context*/, const ::myproject::ConnectPoorRequest* /*request*/, ::myproject::ConnectPoorResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_RegisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_RegisterServer() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_RegisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RegisterServer(::grpc::ServerContext* /*context*/, const ::myproject::RegisterServerRequest* /*request*/, ::myproject::RegisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestRegisterServer(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_UnregisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_UnregisterServer() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_UnregisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status UnregisterServer(::grpc::ServerContext* /*context*/, const ::myproject::UnregisterServerRequest* /*request*/, ::myproject::UnregisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestUnregisterServer(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_GetConnectPoor : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_GetConnectPoor() {
      ::grpc::Service::MarkMethodRaw(2);
    }
    ~WithRawMethod_GetConnectPoor() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetConnectPoor(::grpc::ServerContext* /*context*/, const ::myproject::ConnectPoorRequest* /*request*/, ::myproject::ConnectPoorResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetConnectPoor(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(2, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_RegisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_RegisterServer() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->RegisterServer(context, request, response); }));
    }
    ~WithRawCallbackMethod_RegisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RegisterServer(::grpc::ServerContext* /*context*/, const ::myproject::RegisterServerRequest* /*request*/, ::myproject::RegisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* RegisterServer(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_UnregisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_UnregisterServer() {
      ::grpc::Service::MarkMethodRawCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->UnregisterServer(context, request, response); }));
    }
    ~WithRawCallbackMethod_UnregisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status UnregisterServer(::grpc::ServerContext* /*context*/, const ::myproject::UnregisterServerRequest* /*request*/, ::myproject::UnregisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* UnregisterServer(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_GetConnectPoor : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_GetConnectPoor() {
      ::grpc::Service::MarkMethodRawCallback(2,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->GetConnectPoor(context, request, response); }));
    }
    ~WithRawCallbackMethod_GetConnectPoor() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetConnectPoor(::grpc::ServerContext* /*context*/, const ::myproject::ConnectPoorRequest* /*request*/, ::myproject::ConnectPoorResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* GetConnectPoor(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_RegisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_RegisterServer() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::myproject::RegisterServerRequest, ::myproject::RegisterServerResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::myproject::RegisterServerRequest, ::myproject::RegisterServerResponse>* streamer) {
                       return this->StreamedRegisterServer(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_RegisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status RegisterServer(::grpc::ServerContext* /*context*/, const ::myproject::RegisterServerRequest* /*request*/, ::myproject::RegisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedRegisterServer(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::myproject::RegisterServerRequest,::myproject::RegisterServerResponse>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_UnregisterServer : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_UnregisterServer() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler<
          ::myproject::UnregisterServerRequest, ::myproject::UnregisterServerResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::myproject::UnregisterServerRequest, ::myproject::UnregisterServerResponse>* streamer) {
                       return this->StreamedUnregisterServer(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_UnregisterServer() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status UnregisterServer(::grpc::ServerContext* /*context*/, const ::myproject::UnregisterServerRequest* /*request*/, ::myproject::UnregisterServerResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedUnregisterServer(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::myproject::UnregisterServerRequest,::myproject::UnregisterServerResponse>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_GetConnectPoor : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_GetConnectPoor() {
      ::grpc::Service::MarkMethodStreamed(2,
        new ::grpc::internal::StreamedUnaryHandler<
          ::myproject::ConnectPoorRequest, ::myproject::ConnectPoorResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::myproject::ConnectPoorRequest, ::myproject::ConnectPoorResponse>* streamer) {
                       return this->StreamedGetConnectPoor(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_GetConnectPoor() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status GetConnectPoor(::grpc::ServerContext* /*context*/, const ::myproject::ConnectPoorRequest* /*request*/, ::myproject::ConnectPoorResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedGetConnectPoor(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::myproject::ConnectPoorRequest,::myproject::ConnectPoorResponse>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_RegisterServer<WithStreamedUnaryMethod_UnregisterServer<WithStreamedUnaryMethod_GetConnectPoor<Service > > > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_RegisterServer<WithStreamedUnaryMethod_UnregisterServer<WithStreamedUnaryMethod_GetConnectPoor<Service > > > StreamedService;
};

}  // namespace myproject


#endif  // GRPC_server_5fcentral_2eproto__INCLUDED
