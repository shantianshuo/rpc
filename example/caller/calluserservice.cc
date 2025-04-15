/*
    rpc服务的调用者 -- 业务代码
*/
#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"

int main(int argc, char **argv)
{
    // 整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);
    // 演示调用远程发布的rpc方法Login  调用方需要用到UserServiceRpc_Stub。
    ::fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    // rpc方法的请求参数  调用方设置参数
    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");
    // rpc方法的响应
    fixbug::LoginResponse response;

    // 发起rpc方法的调用  同步的rpc调用过程  MprpcChannel::callmethod
    // controller的作用：把成功或者失败的信息带回来。
    stub.Login(nullptr, &request, &response, nullptr);

    // 同步阻塞方法，等待对端发来的回复。

    
    // 一次rpc调用完成，读调用的结果
    if (0 == response.result().errcode())
    {
        // errorcode == 0 说明没有错误
        std::cout << "rpc login response success:" << response.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
    }

    // 演示调用远程发布的rpc方法Register
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");
    fixbug::RegisterResponse rsp;

    // 以同步的方式发起rpc调用请求，等待返回结果
    stub.Register(nullptr, &req, &rsp, nullptr); 

    // 一次rpc调用完成，读调用的结果
    if (0 == rsp.result().errcode())
    {
        std::cout << "rpc register response success:" << rsp.sucess() << std::endl;
    }
    else
    {
        std::cout << "rpc register response error : " << rsp.result().errmsg() << std::endl;
    }
    
    return 0;
}
