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
namespace rpc_server {

static const char* GatewayServer_method_names[] = {
  "/rpc_server.GatewayServer/Request_forward",
};

std::unique_ptr< GatewayServer::Stub> GatewayServer::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< GatewayServer::Stub> stub(new GatewayServer::Stub(channel, options));
  return stub;
}

GatewayServer::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_Request_forward_(GatewayServer_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status GatewayServer::Stub::Request_forward(::grpc::ClientContext* context, const ::rpc_server::ForwardReq& request, ::rpc_server::ForwardRes* response) {
  return ::grpc::internal::BlockingUnaryCall< ::rpc_server::ForwardReq, ::rpc_server::ForwardRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Request_forward_, context, request, response);
}

void GatewayServer::Stub::async::Request_forward(::grpc::ClientContext* context, const ::rpc_server::ForwardReq* request, ::rpc_server::ForwardRes* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::rpc_server::ForwardReq, ::rpc_server::ForwardRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Request_forward_, context, request, response, std::move(f));
}

void GatewayServer::Stub::async::Request_forward(::grpc::ClientContext* context, const ::rpc_server::ForwardReq* request, ::rpc_server::ForwardRes* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Request_forward_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::ForwardRes>* GatewayServer::Stub::PrepareAsyncRequest_forwardRaw(::grpc::ClientContext* context, const ::rpc_server::ForwardReq& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::rpc_server::ForwardRes, ::rpc_server::ForwardReq, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Request_forward_, context, request);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::ForwardRes>* GatewayServer::Stub::AsyncRequest_forwardRaw(::grpc::ClientContext* context, const ::rpc_server::ForwardReq& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncRequest_forwardRaw(context, request, cq);
  result->StartCall();
  return result;
}

GatewayServer::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      GatewayServer_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< GatewayServer::Service, ::rpc_server::ForwardReq, ::rpc_server::ForwardRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](GatewayServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::rpc_server::ForwardReq* req,
             ::rpc_server::ForwardRes* resp) {
               return service->Request_forward(ctx, req, resp);
             }, this)));
}

GatewayServer::Service::~Service() {
}

::grpc::Status GatewayServer::Service::Request_forward(::grpc::ServerContext* context, const ::rpc_server::ForwardReq* request, ::rpc_server::ForwardRes* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace rpc_server

