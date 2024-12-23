// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: server_gateway.proto
#ifndef GRPC_server_5fgateway_2eproto__INCLUDED
#define GRPC_server_5fgateway_2eproto__INCLUDED

#include "server_gateway.pb.h"

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

class GatewayServer final {
 public:
  static constexpr char const* service_full_name() {
    return "myproject.GatewayServer";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status RequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::myproject::ForwardResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ForwardResponse>> AsyncRequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ForwardResponse>>(AsyncRequestForwardRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ForwardResponse>> PrepareAsyncRequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ForwardResponse>>(PrepareAsyncRequestForwardRaw(context, request, cq));
    }
    // 转发请求
    class async_interface {
     public:
      virtual ~async_interface() {}
      virtual void RequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest* request, ::myproject::ForwardResponse* response, std::function<void(::grpc::Status)>) = 0;
      virtual void RequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest* request, ::myproject::ForwardResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      // 转发请求
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ForwardResponse>* AsyncRequestForwardRaw(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::myproject::ForwardResponse>* PrepareAsyncRequestForwardRaw(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    ::grpc::Status RequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::myproject::ForwardResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::ForwardResponse>> AsyncRequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::ForwardResponse>>(AsyncRequestForwardRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::ForwardResponse>> PrepareAsyncRequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::myproject::ForwardResponse>>(PrepareAsyncRequestForwardRaw(context, request, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void RequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest* request, ::myproject::ForwardResponse* response, std::function<void(::grpc::Status)>) override;
      void RequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest* request, ::myproject::ForwardResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
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
    ::grpc::ClientAsyncResponseReader< ::myproject::ForwardResponse>* AsyncRequestForwardRaw(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::myproject::ForwardResponse>* PrepareAsyncRequestForwardRaw(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_RequestForward_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status RequestForward(::grpc::ServerContext* context, const ::myproject::ForwardRequest* request, ::myproject::ForwardResponse* response);
    // 转发请求
  };
  template <class BaseClass>
  class WithAsyncMethod_RequestForward : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_RequestForward() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_RequestForward() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RequestForward(::grpc::ServerContext* /*context*/, const ::myproject::ForwardRequest* /*request*/, ::myproject::ForwardResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestRequestForward(::grpc::ServerContext* context, ::myproject::ForwardRequest* request, ::grpc::ServerAsyncResponseWriter< ::myproject::ForwardResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_RequestForward<Service > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_RequestForward : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_RequestForward() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::myproject::ForwardRequest, ::myproject::ForwardResponse>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::myproject::ForwardRequest* request, ::myproject::ForwardResponse* response) { return this->RequestForward(context, request, response); }));}
    void SetMessageAllocatorFor_RequestForward(
        ::grpc::MessageAllocator< ::myproject::ForwardRequest, ::myproject::ForwardResponse>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::myproject::ForwardRequest, ::myproject::ForwardResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_RequestForward() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RequestForward(::grpc::ServerContext* /*context*/, const ::myproject::ForwardRequest* /*request*/, ::myproject::ForwardResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* RequestForward(
      ::grpc::CallbackServerContext* /*context*/, const ::myproject::ForwardRequest* /*request*/, ::myproject::ForwardResponse* /*response*/)  { return nullptr; }
  };
  typedef WithCallbackMethod_RequestForward<Service > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_RequestForward : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_RequestForward() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_RequestForward() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RequestForward(::grpc::ServerContext* /*context*/, const ::myproject::ForwardRequest* /*request*/, ::myproject::ForwardResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_RequestForward : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_RequestForward() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_RequestForward() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RequestForward(::grpc::ServerContext* /*context*/, const ::myproject::ForwardRequest* /*request*/, ::myproject::ForwardResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestRequestForward(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_RequestForward : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_RequestForward() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->RequestForward(context, request, response); }));
    }
    ~WithRawCallbackMethod_RequestForward() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RequestForward(::grpc::ServerContext* /*context*/, const ::myproject::ForwardRequest* /*request*/, ::myproject::ForwardResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* RequestForward(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_RequestForward : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_RequestForward() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::myproject::ForwardRequest, ::myproject::ForwardResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::myproject::ForwardRequest, ::myproject::ForwardResponse>* streamer) {
                       return this->StreamedRequestForward(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_RequestForward() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status RequestForward(::grpc::ServerContext* /*context*/, const ::myproject::ForwardRequest* /*request*/, ::myproject::ForwardResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedRequestForward(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::myproject::ForwardRequest,::myproject::ForwardResponse>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_RequestForward<Service > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_RequestForward<Service > StreamedService;
};

}  // namespace myproject


#endif  // GRPC_server_5fgateway_2eproto__INCLUDED
