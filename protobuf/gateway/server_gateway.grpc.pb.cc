// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: server_gateway.proto

#include "server_gateway.pb.h"
#include "server_gateway.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
namespace myproject {

static const char* GatewayService_method_names[] = {
  "/myproject.GatewayService/RequestForward",
};

std::unique_ptr< GatewayService::Stub> GatewayService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< GatewayService::Stub> stub(new GatewayService::Stub(channel, options));
  return stub;
}

GatewayService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_RequestForward_(GatewayService_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status GatewayService::Stub::RequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::myproject::ForwardResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::myproject::ForwardRequest, ::myproject::ForwardResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_RequestForward_, context, request, response);
}

void GatewayService::Stub::async::RequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest* request, ::myproject::ForwardResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::myproject::ForwardRequest, ::myproject::ForwardResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_RequestForward_, context, request, response, std::move(f));
}

void GatewayService::Stub::async::RequestForward(::grpc::ClientContext* context, const ::myproject::ForwardRequest* request, ::myproject::ForwardResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_RequestForward_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::myproject::ForwardResponse>* GatewayService::Stub::PrepareAsyncRequestForwardRaw(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::myproject::ForwardResponse, ::myproject::ForwardRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_RequestForward_, context, request);
}

::grpc::ClientAsyncResponseReader< ::myproject::ForwardResponse>* GatewayService::Stub::AsyncRequestForwardRaw(::grpc::ClientContext* context, const ::myproject::ForwardRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncRequestForwardRaw(context, request, cq);
  result->StartCall();
  return result;
}

GatewayService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      GatewayService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< GatewayService::Service, ::myproject::ForwardRequest, ::myproject::ForwardResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](GatewayService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::myproject::ForwardRequest* req,
             ::myproject::ForwardResponse* resp) {
               return service->RequestForward(ctx, req, resp);
             }, this)));
}

GatewayService::Service::~Service() {
}

::grpc::Status GatewayService::Service::RequestForward(::grpc::ServerContext* context, const ::myproject::ForwardRequest* request, ::myproject::ForwardResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace myproject
