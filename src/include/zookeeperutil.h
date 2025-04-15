#pragma once
/*
    封装的zookeeper客户端类
    用于和zookeeper 的 server 连接，使用之前记得启动zookeeper的服务器。
    为什么引入zookeeper?
        框架支持的rpc通信，必须在一个地方去找到，我现在想调用的rpc服务在哪一台机器上。需要在分布式环境中有一个服务配置中心，来记录分布式节点上，所有发布rpc服务的主机ip地址和端口号
    在rpcprovider中改，在rpcchannel中改
    */
#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// 封装的zk客户端类
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();
    // zkclient启动连接zkserver
    void Start();
    // 在zkserver上根据指定的path创建znode节点
    // 参数：znode节点路径，znode节点数据，数据长度，state=0代表永久性节点，即使超时未发送心跳，也不会删除该节点。
    void Create(const char *path, const char *data, int datalen, int state = 0);
    // 根据参数指定的znode节点路径，或者znode节点的值
    std::string GetData(const char *path);
private:
    // zk的客户端句柄 通过这个句柄，可以操作server了
    zhandle_t *m_zhandle;
};