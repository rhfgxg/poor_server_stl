#ifndef CONST_SERVER_TYPE_H
#define CONST_SERVER_TYPE_H

// 服务器类型
enum ServerType
{
    UNKNOWN = 0;		  // 未知
BATTLE = 1;           // 战斗服务器
CENTRAL = 2;          // 中心服务器
CHAT = 3;             // 聊天服务器
DB = 4;               // 数据库服务器
FILE = 5;             // 文件服务器
GATEWAY = 6;          // 网关服务器
LOGIC = 7;            // 逻辑服务器
LOGIN = 8;            // 登录服务器
MATCHING = 9;         // 匹配服务器
STORE = 10;           // 商城服务器
};

#endif // CONST_SERVER_TYPE_H
