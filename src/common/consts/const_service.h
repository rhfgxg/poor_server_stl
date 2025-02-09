#ifndef CONST_SERVICE_H
#define CONST_SERVICE_H

// 服务相关常量

// 服务类型：客户端与服务器通信的服务类型
enum ServiceType
{
    // REQ_ 请求
    // RES_ 响应
    REQ_LOGIN = 0,      // 登录请求
    RES_LOGIN = 1,      // 登录响应
    REQ_LOGOUT = 2,     // 登出请求
    RES_LOGOUT = 3,     // 登出响应
    REQ_REGISTER = 4,   // 注册请求
    RES_REGISTER = 5,   // 注册响应
    REQ_AUTHENTICATE = 6,  // 令牌验证请求
    RES_AUTHENTICATE = 7  // 令牌验证响应
}


#endif // CONST_SERVICE_H
