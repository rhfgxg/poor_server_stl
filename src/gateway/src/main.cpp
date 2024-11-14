#include <iostream>
#include <boost/asio.hpp>

int main()
{
    std::cout << "����ģ��";

    try
    {
        // ����io_context�������ڹ����첽����
        boost::asio::io_context io_context;

        // ����acceptor���󣬼����˿�1234�ϵ�����
        boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 1234));

        std::cout << "�ȴ��ͻ�������..." << std::endl;

        for (;;)
        {
            // ����socket����������ͻ���ͨ��
            boost::asio::ip::tcp::socket socket(io_context);
            acceptor.accept(socket); // ���ܿͻ�������

            std::cout << "�ͻ������ӳɹ�" << std::endl; // ��ʾ�ͻ������ӳɹ�

        }
    }
    catch (std::exception& e)
    {
        std::cerr << "�쳣: " << e.what() << std::endl; // ���񲢴�ӡ�쳣
    }

    return 0;
}