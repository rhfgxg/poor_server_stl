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
namespace myproject {

static const char* DatabaseServer_method_names[] = {
  "/myproject.DatabaseServer/Create",
  "/myproject.DatabaseServer/Read",
  "/myproject.DatabaseServer/Update",
  "/myproject.DatabaseServer/Delete",
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

::grpc::Status DatabaseServer::Stub::Create(::grpc::ClientContext* context, const ::myproject::CreateRequest& request, ::myproject::CreateResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::myproject::CreateRequest, ::myproject::CreateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Create_, context, request, response);
}

void DatabaseServer::Stub::async::Create(::grpc::ClientContext* context, const ::myproject::CreateRequest* request, ::myproject::CreateResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::myproject::CreateRequest, ::myproject::CreateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Create_, context, request, response, std::move(f));
}

void DatabaseServer::Stub::async::Create(::grpc::ClientContext* context, const ::myproject::CreateRequest* request, ::myproject::CreateResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Create_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::myproject::CreateResponse>* DatabaseServer::Stub::PrepareAsyncCreateRaw(::grpc::ClientContext* context, const ::myproject::CreateRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::myproject::CreateResponse, ::myproject::CreateRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Create_, context, request);
}

::grpc::ClientAsyncResponseReader< ::myproject::CreateResponse>* DatabaseServer::Stub::AsyncCreateRaw(::grpc::ClientContext* context, const ::myproject::CreateRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncCreateRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status DatabaseServer::Stub::Read(::grpc::ClientContext* context, const ::myproject::ReadRequest& request, ::myproject::ReadResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::myproject::ReadRequest, ::myproject::ReadResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Read_, context, request, response);
}

void DatabaseServer::Stub::async::Read(::grpc::ClientContext* context, const ::myproject::ReadRequest* request, ::myproject::ReadResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::myproject::ReadRequest, ::myproject::ReadResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Read_, context, request, response, std::move(f));
}

void DatabaseServer::Stub::async::Read(::grpc::ClientContext* context, const ::myproject::ReadRequest* request, ::myproject::ReadResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Read_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::myproject::ReadResponse>* DatabaseServer::Stub::PrepareAsyncReadRaw(::grpc::ClientContext* context, const ::myproject::ReadRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::myproject::ReadResponse, ::myproject::ReadRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Read_, context, request);
}

::grpc::ClientAsyncResponseReader< ::myproject::ReadResponse>* DatabaseServer::Stub::AsyncReadRaw(::grpc::ClientContext* context, const ::myproject::ReadRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncReadRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status DatabaseServer::Stub::Update(::grpc::ClientContext* context, const ::myproject::UpdateRequest& request, ::myproject::UpdateResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::myproject::UpdateRequest, ::myproject::UpdateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Update_, context, request, response);
}

void DatabaseServer::Stub::async::Update(::grpc::ClientContext* context, const ::myproject::UpdateRequest* request, ::myproject::UpdateResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::myproject::UpdateRequest, ::myproject::UpdateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Update_, context, request, response, std::move(f));
}

void DatabaseServer::Stub::async::Update(::grpc::ClientContext* context, const ::myproject::UpdateRequest* request, ::myproject::UpdateResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Update_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::myproject::UpdateResponse>* DatabaseServer::Stub::PrepareAsyncUpdateRaw(::grpc::ClientContext* context, const ::myproject::UpdateRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::myproject::UpdateResponse, ::myproject::UpdateRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Update_, context, request);
}

::grpc::ClientAsyncResponseReader< ::myproject::UpdateResponse>* DatabaseServer::Stub::AsyncUpdateRaw(::grpc::ClientContext* context, const ::myproject::UpdateRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncUpdateRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status DatabaseServer::Stub::Delete(::grpc::ClientContext* context, const ::myproject::DeleteRequest& request, ::myproject::DeleteResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::myproject::DeleteRequest, ::myproject::DeleteResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_Delete_, context, request, response);
}

void DatabaseServer::Stub::async::Delete(::grpc::ClientContext* context, const ::myproject::DeleteRequest* request, ::myproject::DeleteResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::myproject::DeleteRequest, ::myproject::DeleteResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Delete_, context, request, response, std::move(f));
}

void DatabaseServer::Stub::async::Delete(::grpc::ClientContext* context, const ::myproject::DeleteRequest* request, ::myproject::DeleteResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_Delete_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::myproject::DeleteResponse>* DatabaseServer::Stub::PrepareAsyncDeleteRaw(::grpc::ClientContext* context, const ::myproject::DeleteRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::myproject::DeleteResponse, ::myproject::DeleteRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_Delete_, context, request);
}

::grpc::ClientAsyncResponseReader< ::myproject::DeleteResponse>* DatabaseServer::Stub::AsyncDeleteRaw(::grpc::ClientContext* context, const ::myproject::DeleteRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncDeleteRaw(context, request, cq);
  result->StartCall();
  return result;
}

DatabaseServer::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      DatabaseServer_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< DatabaseServer::Service, ::myproject::CreateRequest, ::myproject::CreateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](DatabaseServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::myproject::CreateRequest* req,
             ::myproject::CreateResponse* resp) {
               return service->Create(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      DatabaseServer_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< DatabaseServer::Service, ::myproject::ReadRequest, ::myproject::ReadResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](DatabaseServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::myproject::ReadRequest* req,
             ::myproject::ReadResponse* resp) {
               return service->Read(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      DatabaseServer_method_names[2],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< DatabaseServer::Service, ::myproject::UpdateRequest, ::myproject::UpdateResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](DatabaseServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::myproject::UpdateRequest* req,
             ::myproject::UpdateResponse* resp) {
               return service->Update(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      DatabaseServer_method_names[3],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< DatabaseServer::Service, ::myproject::DeleteRequest, ::myproject::DeleteResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](DatabaseServer::Service* service,
             ::grpc::ServerContext* ctx,
             const ::myproject::DeleteRequest* req,
             ::myproject::DeleteResponse* resp) {
               return service->Delete(ctx, req, resp);
             }, this)));
}

DatabaseServer::Service::~Service() {
}

::grpc::Status DatabaseServer::Service::Create(::grpc::ServerContext* context, const ::myproject::CreateRequest* request, ::myproject::CreateResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status DatabaseServer::Service::Read(::grpc::ServerContext* context, const ::myproject::ReadRequest* request, ::myproject::ReadResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status DatabaseServer::Service::Update(::grpc::ServerContext* context, const ::myproject::UpdateRequest* request, ::myproject::UpdateResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status DatabaseServer::Service::Delete(::grpc::ServerContext* context, const ::myproject::DeleteRequest* request, ::myproject::DeleteResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace myproject

