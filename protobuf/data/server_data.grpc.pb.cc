// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: server_data.proto

#include "server_data.pb.h"
#include "server_data.grpc.pb.h"

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

static const char* DatabaseServer_method_names[] = {
  "/rpc_server.DatabaseServer/Create",
  "/rpc_server.DatabaseServer/Read",
  "/rpc_server.DatabaseServer/Update",
  "/rpc_server.DatabaseServer/Delete",
};

std::unique_ptr< DatabaseServer::Stub> DatabaseServer::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< DatabaseServer::Stub> stub(new DatabaseServer::Stub(channel, options));
  return stub;
}

DatabaseServer::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_Create_(DatabaseServer_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Read_(DatabaseServer_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Update_(DatabaseServer_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_Delete_(DatabaseServer_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status DatabaseServer::Stub::Create(::grpc::ClientContext* context, const ::rpc_server::CreateReq& request, ::rpc_server::CreateRes* response) {
  return ::grpc::internal::BlockingUnaryCall< ::rpc_server::CreateReq, ::rpc_server::CreateRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Create_, context, request, response);
}

void DatabaseServer::Stub::async::Create(::grpc::ClientContext* context, const ::rpc_server::CreateReq* request, ::rpc_server::CreateRes* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::rpc_server::CreateReq, ::rpc_server::CreateRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Create_, context, request, response, std::move(f));
}

void DatabaseServer::Stub::async::Create(::grpc::ClientContext* context, const ::rpc_server::CreateReq* request, ::rpc_server::CreateRes* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Create_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::CreateRes>* DatabaseServer::Stub::PrepareAsyncCreateRaw(::grpc::ClientContext* context, const ::rpc_server::CreateReq& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::rpc_server::CreateRes, ::rpc_server::CreateReq, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Create_, context, request);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::CreateRes>* DatabaseServer::Stub::AsyncCreateRaw(::grpc::ClientContext* context, const ::rpc_server::CreateReq& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncCreateRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status DatabaseServer::Stub::Read(::grpc::ClientContext* context, const ::rpc_server::ReadReq& request, ::rpc_server::ReadRes* response) {
  return ::grpc::internal::BlockingUnaryCall< ::rpc_server::ReadReq, ::rpc_server::ReadRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Read_, context, request, response);
}

void DatabaseServer::Stub::async::Read(::grpc::ClientContext* context, const ::rpc_server::ReadReq* request, ::rpc_server::ReadRes* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::rpc_server::ReadReq, ::rpc_server::ReadRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Read_, context, request, response, std::move(f));
}

void DatabaseServer::Stub::async::Read(::grpc::ClientContext* context, const ::rpc_server::ReadReq* request, ::rpc_server::ReadRes* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Read_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::ReadRes>* DatabaseServer::Stub::PrepareAsyncReadRaw(::grpc::ClientContext* context, const ::rpc_server::ReadReq& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::rpc_server::ReadRes, ::rpc_server::ReadReq, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Read_, context, request);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::ReadRes>* DatabaseServer::Stub::AsyncReadRaw(::grpc::ClientContext* context, const ::rpc_server::ReadReq& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncReadRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status DatabaseServer::Stub::Update(::grpc::ClientContext* context, const ::rpc_server::UpdateReq& request, ::rpc_server::UpdateRes* response) {
  return ::grpc::internal::BlockingUnaryCall< ::rpc_server::UpdateReq, ::rpc_server::UpdateRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Update_, context, request, response);
}

void DatabaseServer::Stub::async::Update(::grpc::ClientContext* context, const ::rpc_server::UpdateReq* request, ::rpc_server::UpdateRes* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::rpc_server::UpdateReq, ::rpc_server::UpdateRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Update_, context, request, response, std::move(f));
}

void DatabaseServer::Stub::async::Update(::grpc::ClientContext* context, const ::rpc_server::UpdateReq* request, ::rpc_server::UpdateRes* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Update_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::UpdateRes>* DatabaseServer::Stub::PrepareAsyncUpdateRaw(::grpc::ClientContext* context, const ::rpc_server::UpdateReq& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::rpc_server::UpdateRes, ::rpc_server::UpdateReq, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Update_, context, request);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::UpdateRes>* DatabaseServer::Stub::AsyncUpdateRaw(::grpc::ClientContext* context, const ::rpc_server::UpdateReq& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncUpdateRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status DatabaseServer::Stub::Delete(::grpc::ClientContext* context, const ::rpc_server::DeleteReq& request, ::rpc_server::DeleteRes* response) {
  return ::grpc::internal::BlockingUnaryCall< ::rpc_server::DeleteReq, ::rpc_server::DeleteRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Delete_, context, request, response);
}

void DatabaseServer::Stub::async::Delete(::grpc::ClientContext* context, const ::rpc_server::DeleteReq* request, ::rpc_server::DeleteRes* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::rpc_server::DeleteReq, ::rpc_server::DeleteRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Delete_, context, request, response, std::move(f));
}

void DatabaseServer::Stub::async::Delete(::grpc::ClientContext* context, const ::rpc_server::DeleteReq* request, ::rpc_server::DeleteRes* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Delete_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::DeleteRes>* DatabaseServer::Stub::PrepareAsyncDeleteRaw(::grpc::ClientContext* context, const ::rpc_server::DeleteReq& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::rpc_server::DeleteRes, ::rpc_server::DeleteReq, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Delete_, context, request);
}

::grpc::ClientAsyncResponseReader< ::rpc_server::DeleteRes>* DatabaseServer::Stub::AsyncDeleteRaw(::grpc::ClientContext* context, const ::rpc_server::DeleteReq& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncDeleteRaw(context, request, cq);
  result->StartCall();
  return result;
}

DatabaseServer::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      DatabaseServer_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< DatabaseServer::Service, ::rpc_server::CreateReq, ::rpc_server::CreateRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](DatabaseServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::rpc_server::CreateReq* req,
             ::rpc_server::CreateRes* resp) {
               return service->Create(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      DatabaseServer_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< DatabaseServer::Service, ::rpc_server::ReadReq, ::rpc_server::ReadRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](DatabaseServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::rpc_server::ReadReq* req,
             ::rpc_server::ReadRes* resp) {
               return service->Read(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      DatabaseServer_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< DatabaseServer::Service, ::rpc_server::UpdateReq, ::rpc_server::UpdateRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](DatabaseServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::rpc_server::UpdateReq* req,
             ::rpc_server::UpdateRes* resp) {
               return service->Update(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      DatabaseServer_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< DatabaseServer::Service, ::rpc_server::DeleteReq, ::rpc_server::DeleteRes, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](DatabaseServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::rpc_server::DeleteReq* req,
             ::rpc_server::DeleteRes* resp) {
               return service->Delete(ctx, req, resp);
             }, this)));
}

DatabaseServer::Service::~Service() {
}

::grpc::Status DatabaseServer::Service::Create(::grpc::ServerContext* context, const ::rpc_server::CreateReq* request, ::rpc_server::CreateRes* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status DatabaseServer::Service::Read(::grpc::ServerContext* context, const ::rpc_server::ReadReq* request, ::rpc_server::ReadRes* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status DatabaseServer::Service::Update(::grpc::ServerContext* context, const ::rpc_server::UpdateReq* request, ::rpc_server::UpdateRes* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status DatabaseServer::Service::Delete(::grpc::ServerContext* context, const ::rpc_server::DeleteReq* request, ::rpc_server::DeleteRes* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace rpc_server

