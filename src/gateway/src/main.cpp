#include <iostream>
#include <boost/asio.hpp>
#include "./gateway/gateway_service.h"

int main() 
{
    std::cout << "网关模块" << std::endl;

    try 
    {
        /*
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1234));

        std::cout << "等待客户端连接..." << std::endl;

        while (1)
        {
            boost::asio::ip::tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::cout << "客户端连接成功" << std::endl;

            // 读取客户端请求（假设请求是用户名和密码）
            boost::asio::streambuf buffer;
            boost::asio::read_until(socket, buffer, "\n");
            std::istream is(&buffer);
            std::string username, password;
            is >> username >> password;

            // 调用登录服务器
            GatewayService gateway_service(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
            std::string response = gateway_service.Login(username, password);

            // 发送响应给客户端
            boost::asio::write(socket, boost::asio::buffer(response + "\n"));
        */

        // 模拟登录数据
        std::string user_name = "lhw";
        std::string password = "159357";

        // 调用登录服务器
        GatewayService gateway_service(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
        std::string response = gateway_service.Login(user_name, password);

        // 输出响应
        std::cout << "登录响应: " << response << std::endl;

    }
    catch (std::exception& e) 
    {
        std::cerr << "异常: " << e.what() << std::endl;
    }

    return 0;
}
