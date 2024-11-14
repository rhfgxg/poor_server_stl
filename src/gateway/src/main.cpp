#include <iostream>
#include <boost/asio.hpp>

int main()
{
    std::cout << "网关模块";

    try
    {
        // 创建io_context对象，用于管理异步操作
        boost::asio::io_context io_context;

        // 创建acceptor对象，监听端口1234上的连接
        boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1234));

        std::cout << "等待客户端连接..." << std::endl;

        for (;;)
        {
            // 创建socket对象，用于与客户端通信
            boost::asio::ip::tcp::socket socket(io_context);
            acceptor.accept(socket); // 接受客户端连接

            std::cout << "客户端连接成功" << std::endl; // 提示客户端连接成功

        }
    }
    catch (std::exception& e)
    {
        std::cerr << "异常: " << e.what() << std::endl; // 捕获并打印异常
    }

    return 0;
}