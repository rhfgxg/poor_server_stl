#include <iostream>
#include <mysqlx/xdevapi.h>// mysql

mysqlx::Session sql_link();

int main()
{
    mysqlx::Session session = sql_link();

    // 获取数据库中的 Schema 和 Table 对象
    mysqlx::Schema schema = session.getSchema("poor");
    mysqlx::Table table = schema.getTable("users");

    // 关闭连接
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
    std::cout << "数据库链接成功";
}