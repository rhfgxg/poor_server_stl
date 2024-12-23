// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: server_logic.proto
#ifndef GRPC_server_5flogic_2eproto__INCLUDED
#define GRPC_server_5flogic_2eproto__INCLUDED

#include "server_logic.pb.h"

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

class LogicServer final {
 public:
  static constexpr char const* service_full_name() {
    return "myproject.LogicServer";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status ProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::myproject::GameActionResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameActionResponse>> AsyncProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameActionResponse>>(AsyncProcessGameActionRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameActionResponse>> PrepareAsyncProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameActionResponse>>(PrepareAsyncProcessGameActionRaw(context, request, cq));
    }
    // 处理游戏操作
    virtual ::grpc::Status GetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::myproject::GameStateResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameStateResponse>> AsyncGetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameStateResponse>>(AsyncGetGameStateRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameStateResponse>> PrepareAsyncGetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameStateResponse>>(PrepareAsyncGetGameStateRaw(context, request, cq));
    }
    // 获取游戏状态
    class async_interface {
     public:
      virtual ~async_interface() {}
      virtual void ProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest* request, ::myproject::GameActionResponse* response, std::function<void(::grpc::Status)>) = 0;
      virtual void ProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest* request, ::myproject::GameActionResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      // 处理游戏操作
      virtual void GetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest* request, ::myproject::GameStateResponse* response, std::function<void(::grpc::Status)>) = 0;
      virtual void GetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest* request, ::myproject::GameStateResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      // 获取游戏状态
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameActionResponse>* AsyncProcessGameActionRaw(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameActionResponse>* PrepareAsyncProcessGameActionRaw(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameStateResponse>* AsyncGetGameStateRaw(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::GameStateResponse>* PrepareAsyncGetGameStateRaw(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    ::grpc::Status ProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::myproject::GameActionResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::GameActionResponse>> AsyncProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::GameActionResponse>>(AsyncProcessGameActionRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::GameActionResponse>> PrepareAsyncProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::GameActionResponse>>(PrepareAsyncProcessGameActionRaw(context, request, cq));
    }
    ::grpc::Status GetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::myproject::GameStateResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::GameStateResponse>> AsyncGetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::GameStateResponse>>(AsyncGetGameStateRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::GameStateResponse>> PrepareAsyncGetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::GameStateResponse>>(PrepareAsyncGetGameStateRaw(context, request, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void ProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest* request, ::myproject::GameActionResponse* response, std::function<void(::grpc::Status)>) override;
      void ProcessGameAction(::grpc::ClientContext* context, const ::myproject::GameActionRequest* request, ::myproject::GameActionResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
      void GetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest* request, ::myproject::GameStateResponse* response, std::function<void(::grpc::Status)>) override;
      void GetGameState(::grpc::ClientContext* context, const ::myproject::GameStateRequest* request, ::myproject::GameStateResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
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
    ::grpc::ClientAsyncResponseReader< ::myproject::GameActionResponse>* AsyncProcessGameActionRaw(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::myproject::GameActionResponse>* PrepareAsyncProcessGameActionRaw(::grpc::ClientContext* context, const ::myproject::GameActionRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::myproject::GameStateResponse>* AsyncGetGameStateRaw(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::myproject::GameStateResponse>* PrepareAsyncGetGameStateRaw(::grpc::ClientContext* context, const ::myproject::GameStateRequest& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_ProcessGameAction_;
    const ::grpc::internal::RpcMethod rpcmethod_GetGameState_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status ProcessGameAction(::grpc::ServerContext* context, const ::myproject::GameActionRequest* request, ::myproject::GameActionResponse* response);
    // 处理游戏操作
    virtual ::grpc::Status GetGameState(::grpc::ServerContext* context, const ::myproject::GameStateRequest* request, ::myproject::GameStateResponse* response);
    // 获取游戏状态
  };
  template <class BaseClass>
  class WithAsyncMethod_ProcessGameAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_ProcessGameAction() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_ProcessGameAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status ProcessGameAction(::grpc::ServerContext* /*context*/, const ::myproject::GameActionRequest* /*request*/, ::myproject::GameActionResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestProcessGameAction(::grpc::ServerContext* context, ::myproject::GameActionRequest* request, ::grpc::ServerAsyncResponseWriter< ::myproject::GameActionResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_GetGameState : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_GetGameState() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_GetGameState() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetGameState(::grpc::ServerContext* /*context*/, const ::myproject::GameStateRequest* /*request*/, ::myproject::GameStateResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetGameState(::grpc::ServerContext* context, ::myproject::GameStateRequest* request, ::grpc::ServerAsyncResponseWriter< ::myproject::GameStateResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_ProcessGameAction<WithAsyncMethod_GetGameState<Service > > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_ProcessGameAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_ProcessGameAction() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::myproject::GameActionRequest, ::myproject::GameActionResponse>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::myproject::GameActionRequest* request, ::myproject::GameActionResponse* response) { return this->ProcessGameAction(context, request, response); }));}
    void SetMessageAllocatorFor_ProcessGameAction(
        ::grpc::MessageAllocator< ::myproject::GameActionRequest, ::myproject::GameActionResponse>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::myproject::GameActionRequest, ::myproject::GameActionResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_ProcessGameAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status ProcessGameAction(::grpc::ServerContext* /*context*/, const ::myproject::GameActionRequest* /*request*/, ::myproject::GameActionResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* ProcessGameAction(
      ::grpc::CallbackServerContext* /*context*/, const ::myproject::GameActionRequest* /*request*/, ::myproject::GameActionResponse* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_GetGameState : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_GetGameState() {
      ::grpc::Service::MarkMethodCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::myproject::GameStateRequest, ::myproject::GameStateResponse>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::myproject::GameStateRequest* request, ::myproject::GameStateResponse* response) { return this->GetGameState(context, request, response); }));}
    void SetMessageAllocatorFor_GetGameState(
        ::grpc::MessageAllocator< ::myproject::GameStateRequest, ::myproject::GameStateResponse>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(1);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::myproject::GameStateRequest, ::myproject::GameStateResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_GetGameState() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetGameState(::grpc::ServerContext* /*context*/, const ::myproject::GameStateRequest* /*request*/, ::myproject::GameStateResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* GetGameState(
      ::grpc::CallbackServerContext* /*context*/, const ::myproject::GameStateRequest* /*request*/, ::myproject::GameStateResponse* /*response*/)  { return nullptr; }
  };
  typedef WithCallbackMethod_ProcessGameAction<WithCallbackMethod_GetGameState<Service > > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_ProcessGameAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_ProcessGameAction() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_ProcessGameAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status ProcessGameAction(::grpc::ServerContext* /*context*/, const ::myproject::GameActionRequest* /*request*/, ::myproject::GameActionResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_GetGameState : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_GetGameState() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_GetGameState() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetGameState(::grpc::ServerContext* /*context*/, const ::myproject::GameStateRequest* /*request*/, ::myproject::GameStateResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_ProcessGameAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_ProcessGameAction() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_ProcessGameAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status ProcessGameAction(::grpc::ServerContext* /*context*/, const ::myproject::GameActionRequest* /*request*/, ::myproject::GameActionResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestProcessGameAction(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_GetGameState : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_GetGameState() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_GetGameState() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetGameState(::grpc::ServerContext* /*context*/, const ::myproject::GameStateRequest* /*request*/, ::myproject::GameStateResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestGetGameState(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_ProcessGameAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_ProcessGameAction() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->ProcessGameAction(context, request, response); }));
    }
    ~WithRawCallbackMethod_ProcessGameAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status ProcessGameAction(::grpc::ServerContext* /*context*/, const ::myproject::GameActionRequest* /*request*/, ::myproject::GameActionResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* ProcessGameAction(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_GetGameState : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_GetGameState() {
      ::grpc::Service::MarkMethodRawCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->GetGameState(context, request, response); }));
    }
    ~WithRawCallbackMethod_GetGameState() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status GetGameState(::grpc::ServerContext* /*context*/, const ::myproject::GameStateRequest* /*request*/, ::myproject::GameStateResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* GetGameState(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_ProcessGameAction : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_ProcessGameAction() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::myproject::GameActionRequest, ::myproject::GameActionResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::myproject::GameActionRequest, ::myproject::GameActionResponse>* streamer) {
                       return this->StreamedProcessGameAction(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_ProcessGameAction() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status ProcessGameAction(::grpc::ServerContext* /*context*/, const ::myproject::GameActionRequest* /*request*/, ::myproject::GameActionResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedProcessGameAction(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::myproject::GameActionRequest,::myproject::GameActionResponse>* server_unary_streamer) = 0;
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_GetGameState : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_GetGameState() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler<
          ::myproject::GameStateRequest, ::myproject::GameStateResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::myproject::GameStateRequest, ::myproject::GameStateResponse>* streamer) {
                       return this->StreamedGetGameState(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_GetGameState() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status GetGameState(::grpc::ServerContext* /*context*/, const ::myproject::GameStateRequest* /*request*/, ::myproject::GameStateResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedGetGameState(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::myproject::GameStateRequest,::myproject::GameStateResponse>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_ProcessGameAction<WithStreamedUnaryMethod_GetGameState<Service > > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_ProcessGameAction<WithStreamedUnaryMethod_GetGameState<Service > > StreamedService;
};

}  // namespace myproject


#endif  // GRPC_server_5flogic_2eproto__INCLUDED
