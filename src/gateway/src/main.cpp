#include <iostream>
#include <boost/asio.hpp>
#include "./gateway/gateway_service.h"

int main() 
{
    std::cout << "����ģ��" << std::endl;

    try 
    {
        /*
        boost::asio::io_context io_context;
        boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1234));

        std::cout << "�ȴ��ͻ�������..." << std::endl;

        while (1)
        {
            boost::asio::ip::tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::cout << "�ͻ������ӳɹ�" << std::endl;

            // ��ȡ�ͻ������󣨼����������û��������룩
            boost::asio::streambuf buffer;
            boost::asio::read_until(socket, buffer, "\n");
            std::istream is(&buffer);
            std::string username, password;
            is >> username >> password;

            // ���õ�¼������
            GatewayService gateway_service(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
            std::string response = gateway_service.Login(username, password);

            // ������Ӧ���ͻ���
            boost::asio::write(socket, boost::asio::buffer(response + "\n"));
        */

        // ģ���¼����
        std::string user_name = "lhw";
        std::string password = "159357";

        // ���õ�¼������
        GatewayService gateway_service(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
        std::string response = gateway_service.Login(user_name, password);

        // �����Ӧ
        std::cout << "��¼��Ӧ: " << response << std::endl;

    }
    catch (std::exception& e) 
    {
        std::cerr << "�쳣: " << e.what() << std::endl;
    }

    return 0;
}
