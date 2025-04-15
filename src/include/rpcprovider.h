/*
    框架发布服务功能
        成员：m_serviceMap  m_eventLoop
        1.NotifyService函数：传入用户自己写的UserService指针，解析出UserService中方法数量，方法名字
        2.Run函数：
    
*/
#pragma once

#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <google/protobuf/descriptor.h>
#include <string>
#include <functional>
#include <unordered_map> // 方法名和方法的对应关系

// ls框架提供的专门用于发布rpc服务的网络对象类
// 负责数据的序列化反序列化，网络上数据的收发，还有run方法
class RpcProvider
{
public:
    // 这里是框架提供的供外部使用的，可以发布rpc方法的接口，可以接受任意的服务。
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务节点，开始提供rpc远程网络调用服务
    void Run();
private:
    // 需要处理，高并发，添加网络库
    // std::unique_ptr<muduo::net::TcpServer> m_tcpserverPtr; 因为只有run用到了，所以定义为run的局部方法就行了。q：run结束了，指针析构掉了，怎么办？
    muduo::net::EventLoop m_eventLoop;
    // 为什么这里需要一个结构体呢？
    // service服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service; // 保存服务对象
        // 保存服务方法 方法名和方法对应。 
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_methodMap;
    };
    // 注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;
    
    // 回调：如果发生了对应事件，muduo库会调用这两个函数，不用自己调用
    // 新的soket连接回调
    void onConnection(const muduo::net::TcpConnectionPtr& conn);
    // 已经连接的有读写的消息回调
    void onMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);
    // Closure（userservice.cc中的）的回调操作，用于序列化prc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};