// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: server_login.proto
#ifndef GRPC_server_5flogin_2eproto__INCLUDED
#define GRPC_server_5flogin_2eproto__INCLUDED

#include "server_login.pb.h"

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

namespace rpc_server {

class LoginServer final {
 public:
  static constexpr char const* service_full_name() {
    return "rpc_server.LoginServer";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status Login(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::rpc_server::LoginRes* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::LoginRes>> AsyncLogin(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::LoginRes>>(AsyncLoginRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::LoginRes>> PrepareAsyncLogin(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::LoginRes>>(PrepareAsyncLoginRaw(context, request, cq));
    }
    // 登录服务
    virtual ::grpc::Status Register(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::rpc_server::RegisterRes* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::RegisterRes>> AsyncRegister(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::RegisterRes>>(AsyncRegisterRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::RegisterRes>> PrepareAsyncRegister(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::RegisterRes>>(PrepareAsyncRegisterRaw(context, request, cq));
    }
    // 注册服务
    virtual ::grpc::Status Authenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::rpc_server::AuthenticateRes* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::AuthenticateRes>> AsyncAuthenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::AuthenticateRes>>(AsyncAuthenticateRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::AuthenticateRes>> PrepareAsyncAuthenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::AuthenticateRes>>(PrepareAsyncAuthenticateRaw(context, request, cq));
    }
    // 令牌验证服务
    class async_interface {
     public:
      virtual ~async_interface() {}
      virtual void Login(::grpc::ClientContext* context, const ::rpc_server::LoginReq* request, ::rpc_server::LoginRes* response, std::function<void(::grpc::Status)>) = 0;
      virtual void Login(::grpc::ClientContext* context, const ::rpc_server::LoginReq* request, ::rpc_server::LoginRes* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      // 登录服务
      virtual void Register(::grpc::ClientContext* context, const ::rpc_server::RegisterReq* request, ::rpc_server::RegisterRes* response, std::function<void(::grpc::Status)>) = 0;
      virtual void Register(::grpc::ClientContext* context, const ::rpc_server::RegisterReq* request, ::rpc_server::RegisterRes* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      // 注册服务
      virtual void Authenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq* request, ::rpc_server::AuthenticateRes* response, std::function<void(::grpc::Status)>) = 0;
      virtual void Authenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq* request, ::rpc_server::AuthenticateRes* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      // 令牌验证服务
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::LoginRes>* AsyncLoginRaw(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::LoginRes>* PrepareAsyncLoginRaw(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::RegisterRes>* AsyncRegisterRaw(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::RegisterRes>* PrepareAsyncRegisterRaw(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::AuthenticateRes>* AsyncAuthenticateRaw(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::rpc_server::AuthenticateRes>* PrepareAsyncAuthenticateRaw(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    ::grpc::Status Login(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::rpc_server::LoginRes* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::LoginRes>> AsyncLogin(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::LoginRes>>(AsyncLoginRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::LoginRes>> PrepareAsyncLogin(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::LoginRes>>(PrepareAsyncLoginRaw(context, request, cq));
    }
    ::grpc::Status Register(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::rpc_server::RegisterRes* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::RegisterRes>> AsyncRegister(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::RegisterRes>>(AsyncRegisterRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::RegisterRes>> PrepareAsyncRegister(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::RegisterRes>>(PrepareAsyncRegisterRaw(context, request, cq));
    }
    ::grpc::Status Authenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::rpc_server::AuthenticateRes* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::AuthenticateRes>> AsyncAuthenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::AuthenticateRes>>(AsyncAuthenticateRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::AuthenticateRes>> PrepareAsyncAuthenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::rpc_server::AuthenticateRes>>(PrepareAsyncAuthenticateRaw(context, request, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void Login(::grpc::ClientContext* context, const ::rpc_server::LoginReq* request, ::rpc_server::LoginRes* response, std::function<void(::grpc::Status)>) override;
      void Login(::grpc::ClientContext* context, const ::rpc_server::LoginReq* request, ::rpc_server::LoginRes* response, ::grpc::ClientUnaryReactor* reactor) override;
      void Register(::grpc::ClientContext* context, const ::rpc_server::RegisterReq* request, ::rpc_server::RegisterRes* response, std::function<void(::grpc::Status)>) override;
      void Register(::grpc::ClientContext* context, const ::rpc_server::RegisterReq* request, ::rpc_server::RegisterRes* response, ::grpc::ClientUnaryReactor* reactor) override;
      void Authenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq* request, ::rpc_server::AuthenticateRes* response, std::function<void(::grpc::Status)>) override;
      void Authenticate(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq* request, ::rpc_server::AuthenticateRes* response, ::grpc::ClientUnaryReactor* reactor) override;
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
    ::grpc::ClientAsyncResponseReader< ::rpc_server::LoginRes>* AsyncLoginRaw(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::rpc_server::LoginRes>* PrepareAsyncLoginRaw(::grpc::ClientContext* context, const ::rpc_server::LoginReq& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::rpc_server::RegisterRes>* AsyncRegisterRaw(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::rpc_server::RegisterRes>* PrepareAsyncRegisterRaw(::grpc::ClientContext* context, const ::rpc_server::RegisterReq& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::rpc_server::AuthenticateRes>* AsyncAuthenticateRaw(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::rpc_server::AuthenticateRes>* PrepareAsyncAuthenticateRaw(::grpc::ClientContext* context, const ::rpc_server::AuthenticateReq& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_Login_;
    const ::grpc::internal::RpcMethod rpcmethod_Register_;
    const ::grpc::internal::RpcMethod rpcmethod_Authenticate_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status Login(::grpc::ServerContext* context, const ::rpc_server::LoginReq* request, ::rpc_server::LoginRes* response);
    // 登录服务
    virtual ::grpc::Status Register(::grpc::ServerContext* context, const ::rpc_server::RegisterReq* request, ::rpc_server::RegisterRes* response);
    // 注册服务
    virtual ::grpc::Status Authenticate(::grpc::ServerContext* context, const ::rpc_server::AuthenticateReq* request, ::rpc_server::AuthenticateRes* response);
    // 令牌验证服务
  };
  template <class BaseClass>
  class WithAsyncMethod_Login : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_Login() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_Login() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Login(::grpc::ServerContext* /*context*/, const ::rpc_server::LoginReq* /*request*/, ::rpc_server::LoginRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestLogin(::grpc::ServerContext* context, ::rpc_server::LoginReq* request, ::grpc::ServerAsyncResponseWriter< ::rpc_server::LoginRes>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_Register : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_Register() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_Register() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Register(::grpc::ServerContext* /*context*/, const ::rpc_server::RegisterReq* /*request*/, ::rpc_server::RegisterRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestRegister(::grpc::ServerContext* context, ::rpc_server::RegisterReq* request, ::grpc::ServerAsyncResponseWriter< ::rpc_server::RegisterRes>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_Authenticate : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_Authenticate() {
      ::grpc::Service::MarkMethodAsync(2);
    }
    ~WithAsyncMethod_Authenticate() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Authenticate(::grpc::ServerContext* /*context*/, const ::rpc_server::AuthenticateReq* /*request*/, ::rpc_server::AuthenticateRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestAuthenticate(::grpc::ServerContext* context, ::rpc_server::AuthenticateReq* request, ::grpc::ServerAsyncResponseWriter< ::rpc_server::AuthenticateRes>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(2, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_Login<WithAsyncMethod_Register<WithAsyncMethod_Authenticate<Service > > > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_Login : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_Login() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::rpc_server::LoginReq, ::rpc_server::LoginRes>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::rpc_server::LoginReq* request, ::rpc_server::LoginRes* response) { return this->Login(context, request, response); }));}
    void SetMessageAllocatorFor_Login(
        ::grpc::MessageAllocator< ::rpc_server::LoginReq, ::rpc_server::LoginRes>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::rpc_server::LoginReq, ::rpc_server::LoginRes>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_Login() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Login(::grpc::ServerContext* /*context*/, const ::rpc_server::LoginReq* /*request*/, ::rpc_server::LoginRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* Login(
      ::grpc::CallbackServerContext* /*context*/, const ::rpc_server::LoginReq* /*request*/, ::rpc_server::LoginRes* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_Register : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_Register() {
      ::grpc::Service::MarkMethodCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::rpc_server::RegisterReq, ::rpc_server::RegisterRes>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::rpc_server::RegisterReq* request, ::rpc_server::RegisterRes* response) { return this->Register(context, request, response); }));}
    void SetMessageAllocatorFor_Register(
        ::grpc::MessageAllocator< ::rpc_server::RegisterReq, ::rpc_server::RegisterRes>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(1);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::rpc_server::RegisterReq, ::rpc_server::RegisterRes>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_Register() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Register(::grpc::ServerContext* /*context*/, const ::rpc_server::RegisterReq* /*request*/, ::rpc_server::RegisterRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* Register(
      ::grpc::CallbackServerContext* /*context*/, const ::rpc_server::RegisterReq* /*request*/, ::rpc_server::RegisterRes* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_Authenticate : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_Authenticate() {
      ::grpc::Service::MarkMethodCallback(2,
          new ::grpc::internal::CallbackUnaryHandler< ::rpc_server::AuthenticateReq, ::rpc_server::AuthenticateRes>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::rpc_server::AuthenticateReq* request, ::rpc_server::AuthenticateRes* response) { return this->Authenticate(context, request, response); }));}
    void SetMessageAllocatorFor_Authenticate(
        ::grpc::MessageAllocator< ::rpc_server::AuthenticateReq, ::rpc_server::AuthenticateRes>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(2);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::rpc_server::AuthenticateReq, ::rpc_server::AuthenticateRes>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_Authenticate() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Authenticate(::grpc::ServerContext* /*context*/, const ::rpc_server::AuthenticateReq* /*request*/, ::rpc_server::AuthenticateRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* Authenticate(
      ::grpc::CallbackServerContext* /*context*/, const ::rpc_server::AuthenticateReq* /*request*/, ::rpc_server::AuthenticateRes* /*response*/)  { return nullptr; }
  };
  typedef WithCallbackMethod_Login<WithCallbackMethod_Register<WithCallbackMethod_Authenticate<Service > > > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_Login : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_Login() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_Login() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Login(::grpc::ServerContext* /*context*/, const ::rpc_server::LoginReq* /*request*/, ::rpc_server::LoginRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_Register : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_Register() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_Register() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Register(::grpc::ServerContext* /*context*/, const ::rpc_server::RegisterReq* /*request*/, ::rpc_server::RegisterRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_Authenticate : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_Authenticate() {
      ::grpc::Service::MarkMethodGeneric(2);
    }
    ~WithGenericMethod_Authenticate() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Authenticate(::grpc::ServerContext* /*context*/, const ::rpc_server::AuthenticateReq* /*request*/, ::rpc_server::AuthenticateRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_Login : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_Login() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_Login() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Login(::grpc::ServerContext* /*context*/, const ::rpc_server::LoginReq* /*request*/, ::rpc_server::LoginRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestLogin(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_Register : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_Register() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_Register() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Register(::grpc::ServerContext* /*context*/, const ::rpc_server::RegisterReq* /*request*/, ::rpc_server::RegisterRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestRegister(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_Authenticate : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_Authenticate() {
      ::grpc::Service::MarkMethodRaw(2);
    }
    ~WithRawMethod_Authenticate() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Authenticate(::grpc::ServerContext* /*context*/, const ::rpc_server::AuthenticateReq* /*request*/, ::rpc_server::AuthenticateRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestAuthenticate(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(2, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_Login : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_Login() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->Login(context, request, response); }));
    }
    ~WithRawCallbackMethod_Login() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Login(::grpc::ServerContext* /*context*/, const ::rpc_server::LoginReq* /*request*/, ::rpc_server::LoginRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* Login(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_Register : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_Register() {
      ::grpc::Service::MarkMethodRawCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->Register(context, request, response); }));
    }
    ~WithRawCallbackMethod_Register() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Register(::grpc::ServerContext* /*context*/, const ::rpc_server::RegisterReq* /*request*/, ::rpc_server::RegisterRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* Register(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_Authenticate : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_Authenticate() {
      ::grpc::Service::MarkMethodRawCallback(2,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->Authenticate(context, request, response); }));
    }
    ~WithRawCallbackMethod_Authenticate() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status Authenticate(::grpc::ServerContext* /*context*/, const ::rpc_server::AuthenticateReq* /*request*/, ::rpc_server::AuthenticateRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* Authenticate(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_Login : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_Login() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::rpc_server::LoginReq, ::rpc_server::LoginRes>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::rpc_server::LoginReq, ::rpc_server::LoginRes>* streamer) {
                       return this->StreamedLogin(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_Login() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status Login(::grpc::ServerContext* /*context*/, const ::rpc_server::LoginReq* /*request*/, ::rpc_server::LoginRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedLogin(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::rpc_server::LoginReq,::rpc_server::LoginRes>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_Register : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_Register() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler<
          ::rpc_server::RegisterReq, ::rpc_server::RegisterRes>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::rpc_server::RegisterReq, ::rpc_server::RegisterRes>* streamer) {
                       return this->StreamedRegister(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_Register() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status Register(::grpc::ServerContext* /*context*/, const ::rpc_server::RegisterReq* /*request*/, ::rpc_server::RegisterRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedRegister(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::rpc_server::RegisterReq,::rpc_server::RegisterRes>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_Authenticate : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_Authenticate() {
      ::grpc::Service::MarkMethodStreamed(2,
        new ::grpc::internal::StreamedUnaryHandler<
          ::rpc_server::AuthenticateReq, ::rpc_server::AuthenticateRes>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::rpc_server::AuthenticateReq, ::rpc_server::AuthenticateRes>* streamer) {
                       return this->StreamedAuthenticate(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_Authenticate() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status Authenticate(::grpc::ServerContext* /*context*/, const ::rpc_server::AuthenticateReq* /*request*/, ::rpc_server::AuthenticateRes* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedAuthenticate(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::rpc_server::AuthenticateReq,::rpc_server::AuthenticateRes>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_Login<WithStreamedUnaryMethod_Register<WithStreamedUnaryMethod_Authenticate<Service > > > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_Login<WithStreamedUnaryMethod_Register<WithStreamedUnaryMethod_Authenticate<Service > > > StreamedService;
};

}  // namespace rpc_server


#endif  // GRPC_server_5flogin_2eproto__INCLUDED
