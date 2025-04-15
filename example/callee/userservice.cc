/*
    rpc服务的提供者
    发布方法步骤：
        1.在proto中构建相应的方法，设置参数类型。
        2.构建UserService类继承于UserServiceRpc重写自己构建的方法
        3.重写的方法中，调用本地的方法。
*/
#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
// UserService 有Login GetFriendlists
class UserService : public fixbug::UserServiceRpc //使用在rpc服务的发送端，rpc服务的提供者。
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing Login service: Login..." << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }
    /*
    重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    1. caller   ===>   Login(LoginRequest)  => muduo =>   callee 
    2. callee   ===>    Login(LoginRequest)  => 交到下面重写的这个Login方法上了
    */
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing Register service: Register..." << std::endl;
        std::cout << "id:" << id << " name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }
    void Login(::google::protobuf::RpcController* controller,
        const ::fixbug::LoginRequest* request, // 参数
        ::fixbug::LoginResponse* response, // 返回值
        ::google::protobuf::Closure* done) // 回调 Closure纯虚函数
    {
        // 框架给业务上报了请求参数LoginRequest，应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd); 

        // 把响应写入  包括错误码、错误消息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        // 执行回调操作 response填写完了，通知框架应该把它返回对端  执行响应对象数据的序列化和网络发送（都是由框架来完成的）
        done->Run();
    }
    void Register(::google::protobuf::RpcController* controller,
        const ::fixbug::RegisterRequest* request,
        ::fixbug::RegisterResponse* response,
        ::google::protobuf::Closure* done)
    {
        std::string name = request->name();
        std::string pwd = request->pwd();
        uint32_t id = request->id();
        bool register_result = Register(id, name, pwd); 
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(register_result);
        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 将本地服务发布到框架。
    // 调用框架的初始化操作  
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象。把UserService对象发布到rpc节点上
    RpcProvider provider; //负责数据的序列化反序列化，网络上数据的收发，还有run方法
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点   Run以后，进程进入阻塞状态，等待远程的rpc调用请求  启动服务器。
    provider.Run();
    return 0;
}