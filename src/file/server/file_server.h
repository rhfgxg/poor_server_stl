#ifndef FILE_SERVER_H
#define FILE_SERVER_H

#include "common.grpc.pb.h"
#include "server_file.grpc.pb.h"
#include "base_server.h"
#include "logger_manager.h"

#include <grpcpp/grpcpp.h>
#include <memory>

/**
 * @brief 文件服务器实现类（重构版本）
 * 
 * 继承自 BaseServer，自动获得：
 * - 线程池管理（ThreadManager）
 * - 服务器注册/注销
 * - 心跳发送
 * - 日志管理
 * 
 * 只需实现业务逻辑：
 * - 文件上传/下载
 * - 文件删除/列表
 */
class FileServerImpl final : public BaseServer, public rpc_server::FileServer::Service
{
public:
    FileServerImpl(LoggerManager& logger_manager_, const std::string& address, const std::string& port);
    ~FileServerImpl() override;

    // gRPC 对外接口
    grpc::Status Transmission_ready(grpc::ServerContext* context, const rpc_server::TransmissionReadyReq* req, rpc_server::TransmissionReadyRes* res) override;
    grpc::Status Upload(grpc::ServerContext* context, grpc::ServerReader<rpc_server::UploadReq>* reader, rpc_server::UploadRes* res) override;
    grpc::Status Download(grpc::ServerContext* context, const rpc_server::DownloadReq* req, grpc::ServerWriter<rpc_server::DownloadRes>* writer) override;
    grpc::Status Delete(grpc::ServerContext* context, const rpc_server::DeleteFileReq* req, rpc_server::DeleteFileRes* res) override;
    grpc::Status ListFiles(grpc::ServerContext* context, const rpc_server::ListFilesReq* req, rpc_server::ListFilesRes* res) override;

protected:
    // BaseServer 钩子方法
    bool on_start() override;
    void on_stop() override;

private:
    // 文件操作处理函数
    void Handle_transmission_ready(const rpc_server::TransmissionReadyReq* req, rpc_server::TransmissionReadyRes* res);
    grpc::Status Handle_upload(grpc::ServerReader<rpc_server::UploadReq>* reader, rpc_server::UploadRes* res);
    grpc::Status Handle_download(const rpc_server::DownloadReq* req, grpc::ServerWriter<rpc_server::DownloadRes>* writer);
    void Handle_delete(const rpc_server::DeleteFileReq* req, rpc_server::DeleteFileRes* res);
    void Handle_list_files(const rpc_server::ListFilesReq* req, rpc_server::ListFilesRes* res);
};

#endif // FILE_SERVER_H
