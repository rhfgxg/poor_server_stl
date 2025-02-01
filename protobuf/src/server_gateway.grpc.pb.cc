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
  "/rpc_server.GatewayServer/Get_file_server_address",
  "/rpc_server.GatewayServer/Client_heartbeat",
  "/rpc_server.GatewayServer/Get_gateway_pool",
};

std::unique_ptr< GatewayServer::Stub> GatewayServer::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< GatewayServer::Stub> stub(new GatewayServer::Stub(channel, options));
  return stub;
}

GatewayServer::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_Request_forward_(GatewayServer_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Get_file_server_address_(GatewayServer_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Client_heartbeat_(GatewayServer_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Get_gateway_pool_(GatewayServer_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
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

::grpc::Status GatewayServer::Stub::Get_file_server_address(::grpc::ClientContext* context, const ::rpc_server::GetFileServerAddressReq& request, ::rpc_server::GetFileServerAddressRes* response) {
  return ::grpc::internal::BlockingUnaryCall< ::rpc_server::GetFileServerAddressReq, ::rpc_server::GetFileServerAddressRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Get_file_server_address_, context, request, response);
}

void GatewayServer::Stub::async::Get_file_server_address(::grpc::ClientContext* context, const ::rpc_server::GetFileServerAddressReq* request, ::rpc_server::GetFileServerAddressRes* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::rpc_server::GetFileServerAddressReq, ::rpc_server::GetFileServerAddressRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Get_file_server_address_, context, request, response, std::move(f));
}

void GatewayServer::Stub::async::Get_file_server_address(::grpc::ClientContext* context, const ::rpc_server::GetFileServerAddressReq* request, ::rpc_server::GetFileServerAddressRes* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Get_file_server_address_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::GetFileServerAddressRes>* GatewayServer::Stub::PrepareAsyncGet_file_server_addressRaw(::grpc::ClientContext* context, const ::rpc_server::GetFileServerAddressReq& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::rpc_server::GetFileServerAddressRes, ::rpc_server::GetFileServerAddressReq, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Get_file_server_address_, context, request);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::GetFileServerAddressRes>* GatewayServer::Stub::AsyncGet_file_server_addressRaw(::grpc::ClientContext* context, const ::rpc_server::GetFileServerAddressReq& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGet_file_server_addressRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status GatewayServer::Stub::Client_heartbeat(::grpc::ClientContext* context, const ::rpc_server::ClientHeartbeatReq& request, ::rpc_server::ClientHeartbeatRes* response) {
  return ::grpc::internal::BlockingUnaryCall< ::rpc_server::ClientHeartbeatReq, ::rpc_server::ClientHeartbeatRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Client_heartbeat_, context, request, response);
}

void GatewayServer::Stub::async::Client_heartbeat(::grpc::ClientContext* context, const ::rpc_server::ClientHeartbeatReq* request, ::rpc_server::ClientHeartbeatRes* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::rpc_server::ClientHeartbeatReq, ::rpc_server::ClientHeartbeatRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Client_heartbeat_, context, request, response, std::move(f));
}

void GatewayServer::Stub::async::Client_heartbeat(::grpc::ClientContext* context, const ::rpc_server::ClientHeartbeatReq* request, ::rpc_server::ClientHeartbeatRes* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Client_heartbeat_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::ClientHeartbeatRes>* GatewayServer::Stub::PrepareAsyncClient_heartbeatRaw(::grpc::ClientContext* context, const ::rpc_server::ClientHeartbeatReq& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::rpc_server::ClientHeartbeatRes, ::rpc_server::ClientHeartbeatReq, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Client_heartbeat_, context, request);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::ClientHeartbeatRes>* GatewayServer::Stub::AsyncClient_heartbeatRaw(::grpc::ClientContext* context, const ::rpc_server::ClientHeartbeatReq& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncClient_heartbeatRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status GatewayServer::Stub::Get_gateway_pool(::grpc::ClientContext* context, const ::rpc_server::GetGatewayPoolReq& request, ::rpc_server::GetGatewayPoolRes* response) {
  return ::grpc::internal::BlockingUnaryCall< ::rpc_server::GetGatewayPoolReq, ::rpc_server::GetGatewayPoolRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Get_gateway_pool_, context, request, response);
}

void GatewayServer::Stub::async::Get_gateway_pool(::grpc::ClientContext* context, const ::rpc_server::GetGatewayPoolReq* request, ::rpc_server::GetGatewayPoolRes* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::rpc_server::GetGatewayPoolReq, ::rpc_server::GetGatewayPoolRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Get_gateway_pool_, context, request, response, std::move(f));
}

void GatewayServer::Stub::async::Get_gateway_pool(::grpc::ClientContext* context, const ::rpc_server::GetGatewayPoolReq* request, ::rpc_server::GetGatewayPoolRes* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Get_gateway_pool_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::GetGatewayPoolRes>* GatewayServer::Stub::PrepareAsyncGet_gateway_poolRaw(::grpc::ClientContext* context, const ::rpc_server::GetGatewayPoolReq& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::rpc_server::GetGatewayPoolRes, ::rpc_server::GetGatewayPoolReq, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Get_gateway_pool_, context, request);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::GetGatewayPoolRes>* GatewayServer::Stub::AsyncGet_gateway_poolRaw(::grpc::ClientContext* context, const ::rpc_server::GetGatewayPoolReq& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGet_gateway_poolRaw(context, request, cq);
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
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      GatewayServer_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< GatewayServer::Service, ::rpc_server::GetFileServerAddressReq, ::rpc_server::GetFileServerAddressRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](GatewayServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::rpc_server::GetFileServerAddressReq* req,
             ::rpc_server::GetFileServerAddressRes* resp) {
               return service->Get_file_server_address(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      GatewayServer_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< GatewayServer::Service, ::rpc_server::ClientHeartbeatReq, ::rpc_server::ClientHeartbeatRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](GatewayServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::rpc_server::ClientHeartbeatReq* req,
             ::rpc_server::ClientHeartbeatRes* resp) {
               return service->Client_heartbeat(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      GatewayServer_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< GatewayServer::Service, ::rpc_server::GetGatewayPoolReq, ::rpc_server::GetGatewayPoolRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](GatewayServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::rpc_server::GetGatewayPoolReq* req,
             ::rpc_server::GetGatewayPoolRes* resp) {
               return service->Get_gateway_pool(ctx, req, resp);
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

::grpc::Status GatewayServer::Service::Get_file_server_address(::grpc::ServerContext* context, const ::rpc_server::GetFileServerAddressReq* request, ::rpc_server::GetFileServerAddressRes* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status GatewayServer::Service::Client_heartbeat(::grpc::ServerContext* context, const ::rpc_server::ClientHeartbeatReq* request, ::rpc_server::ClientHeartbeatRes* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status GatewayServer::Service::Get_gateway_pool(::grpc::ServerContext* context, const ::rpc_server::GetGatewayPoolReq* request, ::rpc_server::GetGatewayPoolRes* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace rpc_server

