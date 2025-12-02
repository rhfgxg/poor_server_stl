#include "file_server.h"
#include <fstream>

// 构造函数 - 继承 BaseServer
FileServerImpl::FileServerImpl(LoggerManager& logger_manager_, const std::string& address, const std::string& port)
    : BaseServer(rpc_server::ServerType::FILE, address, port, logger_manager_, 4)  // 4 个线程
{
    // 业务逻辑将在 on_start() 中初始化
}

// 析构函数
FileServerImpl::~FileServerImpl()
{
    log_shutdown("FileServer stopped");
}

// ==================== BaseServer 钩子方法 ====================

bool FileServerImpl::on_start()
{
    log_startup("FileServer initializing...");
    
    // TODO: 这里可以添加文件服务器特定的初始化
    // 例如：创建文件存储目录、检查磁盘空间等
    
    log_startup("FileServer initialized successfully");
    return true;
}

void FileServerImpl::on_stop()
{
    log_shutdown("FileServer shutting down...");
    
    // TODO: 这里可以添加清理逻辑
    
    log_shutdown("FileServer shutdown complete");
}

// ==================== gRPC 服务接口 ====================

grpc::Status FileServerImpl::Transmission_ready(grpc::ServerContext* context [[maybe_unused]], const rpc_server::TransmissionReadyReq* req, rpc_server::TransmissionReadyRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_transmission_ready(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status FileServerImpl::Upload(grpc::ServerContext* context [[maybe_unused]], grpc::ServerReader<rpc_server::UploadReq>* reader, rpc_server::UploadRes* res)
{
    grpc::Status status;
    auto task_future = submit_task([this, &status, reader, res]() {
        status = Handle_upload(reader, res);
    });
    task_future.get();
    return status;
}

grpc::Status FileServerImpl::Download(grpc::ServerContext* context [[maybe_unused]], const rpc_server::DownloadReq* req, grpc::ServerWriter<rpc_server::DownloadRes>* writer)
{
    grpc::Status status;
    auto task_future = submit_task([this, &status, req, writer]() {
        status = Handle_download(req, writer);
    });
    task_future.get();
    return status;
}

grpc::Status FileServerImpl::Delete(grpc::ServerContext* context [[maybe_unused]], const rpc_server::DeleteFileReq* req, rpc_server::DeleteFileRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_delete(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

grpc::Status FileServerImpl::ListFiles(grpc::ServerContext* context [[maybe_unused]], const rpc_server::ListFilesReq* req, rpc_server::ListFilesRes* res)
{
    auto task_future = submit_task([this, req, res]() {
        Handle_list_files(req, res);
    });
    task_future.get();
    return grpc::Status::OK;
}

// ==================== 文件操作处理函数 ====================

void FileServerImpl::Handle_transmission_ready(const rpc_server::TransmissionReadyReq* req, rpc_server::TransmissionReadyRes* res)
{
    std::string account = req->account();
    std::string token = req->token();
    std::string file_name = req->file_name();

    // 返回文件服务器地址与端口
    res->set_file_server_address(server_address_);
    res->set_file_server_port(server_port_);

    res->set_success(true);
    res->set_message("successful");
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Transmission_ready successful: {}", account);
}

grpc::Status FileServerImpl::Handle_upload(grpc::ServerReader<rpc_server::UploadReq>* reader, rpc_server::UploadRes* res)
{
    rpc_server::UploadReq request;
    std::ofstream file;
    bool first_chunk = true;

    while (reader->Read(&request))
    {
        if (first_chunk)
        {
            std::string file_name = request.file_name();
            std::string file_url = "./files/" + file_name;
            file.open(file_url, std::ios::binary);
            if (!file.is_open())
            {
                get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("Failed to open file: {}", file_url);
                return grpc::Status(grpc::StatusCode::INTERNAL, "Failed to open file");
            }
            first_chunk = false;
        }
        file.write(request.file_data().data(), request.file_data().size());
    }

    file.close();
    res->set_success(true);
    res->set_message("Upload successful");
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Upload successful");
    return grpc::Status::OK;
}

grpc::Status FileServerImpl::Handle_download(const rpc_server::DownloadReq* req, grpc::ServerWriter<rpc_server::DownloadRes>* writer)
{
    std::string account = req->account();
    std::string file_name = req->file_name();
    std::string file_url = "./files/" + file_name;

    std::ifstream file(file_url, std::ios::binary);
    if (!file.is_open())
    {
        get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->error("Failed to open file: {}", file_url);
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "File not found");
    }

    char buffer[1024];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0)
    {
        rpc_server::DownloadRes response;
        response.set_file_data(buffer, file.gcount());
        writer->Write(response);
    }

    file.close();
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Download successful: {}", account);
    return grpc::Status::OK;
}

void FileServerImpl::Handle_delete(const rpc_server::DeleteFileReq* req, rpc_server::DeleteFileRes* res)
{
    std::string account = req->account();
    std::string file_name = req->file_name();

    res->set_success(true);
    res->set_message("successful");
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("Delete successful: {}", account);
}

void FileServerImpl::Handle_list_files(const rpc_server::ListFilesReq* req, rpc_server::ListFilesRes* res)
{
    std::string account = req->account();

    res->set_success(true);
    res->set_message("successful");
    get_logger(poor::LogCategory::APPLICATION_ACTIVITY)->info("File_list successful: {}", account);
}
