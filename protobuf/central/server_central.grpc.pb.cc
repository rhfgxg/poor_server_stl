// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: server_central.proto

#include "server_central.pb.h"
#include "server_central.grpc.pb.h"

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

static const char* CentralServer_method_names[] = {
  "/myproject.CentralServer/RegisterServer",
  "/myproject.CentralServer/UnregisterServer",
  "/myproject.CentralServer/GetServerInfo",
};

std::unique_ptr< CentralServer::Stub> CentralServer::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< CentralServer::Stub> stub(new CentralServer::Stub(channel, options));
  return stub;
}

CentralServer::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_RegisterServer_(CentralServer_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_UnregisterServer_(CentralServer_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetServerInfo_(CentralServer_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status CentralServer::Stub::RegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::myproject::RegisterServerResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::myproject::RegisterServerRequest, ::myproject::RegisterServerResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_RegisterServer_, context, request, response);
}

void CentralServer::Stub::async::RegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest* request, ::myproject::RegisterServerResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::myproject::RegisterServerRequest, ::myproject::RegisterServerResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_RegisterServer_, context, request, response, std::move(f));
}

void CentralServer::Stub::async::RegisterServer(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest* request, ::myproject::RegisterServerResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_RegisterServer_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::myproject::RegisterServerResponse>* CentralServer::Stub::PrepareAsyncRegisterServerRaw(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::myproject::RegisterServerResponse, ::myproject::RegisterServerRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_RegisterServer_, context, request);
}

::grpc::ClientAsyncResponseReader< ::myproject::RegisterServerResponse>* CentralServer::Stub::AsyncRegisterServerRaw(::grpc::ClientContext* context, const ::myproject::RegisterServerRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncRegisterServerRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status CentralServer::Stub::UnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::myproject::UnregisterServerResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::myproject::UnregisterServerRequest, ::myproject::UnregisterServerResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_UnregisterServer_, context, request, response);
}

void CentralServer::Stub::async::UnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest* request, ::myproject::UnregisterServerResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::myproject::UnregisterServerRequest, ::myproject::UnregisterServerResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_UnregisterServer_, context, request, response, std::move(f));
}

void CentralServer::Stub::async::UnregisterServer(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest* request, ::myproject::UnregisterServerResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_UnregisterServer_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::myproject::UnregisterServerResponse>* CentralServer::Stub::PrepareAsyncUnregisterServerRaw(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::myproject::UnregisterServerResponse, ::myproject::UnregisterServerRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_UnregisterServer_, context, request);
}

::grpc::ClientAsyncResponseReader< ::myproject::UnregisterServerResponse>* CentralServer::Stub::AsyncUnregisterServerRaw(::grpc::ClientContext* context, const ::myproject::UnregisterServerRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncUnregisterServerRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status CentralServer::Stub::GetServerInfo(::grpc::ClientContext* context, const ::myproject::ServerInfoRequest& request, ::myproject::ServerInfoResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::myproject::ServerInfoRequest, ::myproject::ServerInfoResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetServerInfo_, context, request, response);
}

void CentralServer::Stub::async::GetServerInfo(::grpc::ClientContext* context, const ::myproject::ServerInfoRequest* request, ::myproject::ServerInfoResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::myproject::ServerInfoRequest, ::myproject::ServerInfoResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetServerInfo_, context, request, response, std::move(f));
}

void CentralServer::Stub::async::GetServerInfo(::grpc::ClientContext* context, const ::myproject::ServerInfoRequest* request, ::myproject::ServerInfoResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetServerInfo_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::myproject::ServerInfoResponse>* CentralServer::Stub::PrepareAsyncGetServerInfoRaw(::grpc::ClientContext* context, const ::myproject::ServerInfoRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::myproject::ServerInfoResponse, ::myproject::ServerInfoRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetServerInfo_, context, request);
}

::grpc::ClientAsyncResponseReader< ::myproject::ServerInfoResponse>* CentralServer::Stub::AsyncGetServerInfoRaw(::grpc::ClientContext* context, const ::myproject::ServerInfoRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetServerInfoRaw(context, request, cq);
  result->StartCall();
  return result;
}

CentralServer::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      CentralServer_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< CentralServer::Service, ::myproject::RegisterServerRequest, ::myproject::RegisterServerResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](CentralServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::myproject::RegisterServerRequest* req,
             ::myproject::RegisterServerResponse* resp) {
               return service->RegisterServer(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      CentralServer_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< CentralServer::Service, ::myproject::UnregisterServerRequest, ::myproject::UnregisterServerResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](CentralServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::myproject::UnregisterServerRequest* req,
             ::myproject::UnregisterServerResponse* resp) {
               return service->UnregisterServer(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      CentralServer_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< CentralServer::Service, ::myproject::ServerInfoRequest, ::myproject::ServerInfoResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](CentralServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::myproject::ServerInfoRequest* req,
             ::myproject::ServerInfoResponse* resp) {
               return service->GetServerInfo(ctx, req, resp);
             }, this)));
}

CentralServer::Service::~Service() {
}

::grpc::Status CentralServer::Service::RegisterServer(::grpc::ServerContext* context, const ::myproject::RegisterServerRequest* request, ::myproject::RegisterServerResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status CentralServer::Service::UnregisterServer(::grpc::ServerContext* context, const ::myproject::UnregisterServerRequest* request, ::myproject::UnregisterServerResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status CentralServer::Service::GetServerInfo(::grpc::ServerContext* context, const ::myproject::ServerInfoRequest* request, ::myproject::ServerInfoResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace myproject

