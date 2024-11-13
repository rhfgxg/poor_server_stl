#include <iostream>
#include <mysqlx/xdevapi.h>// mysql

mysqlx::Session sql_link();

int main()
{
    mysqlx::Session session = sql_link();

    // ��ȡ���ݿ��е� Schema �� Table ����
    mysqlx::Schema schema = session.getSchema("poor");
    mysqlx::Table table = schema.getTable("users");

    // �ر�����
    session.close();

    return 0;
}

mysqlx::Session sql_link()
{
    try {
        std::cout << "aaaa\n";
        mysqlx::SessionSettings option("localhost", 33060, "root", "159357");
        mysqlx::Session session(option);


        return session;
    }
    catch (const mysqlx::Error& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        return;
    }
    catch (std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return;
    }
    catch (...) {
        std::cerr << "Unknown error occurred!" << std::endl;
        return;
    }
    std::cout << "���ݿ����ӳɹ�";
}