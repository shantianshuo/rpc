/*
    框架发布服务功能
*/
#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

// 这里是框架提供的供外部使用的，可以发布rpc方法的接口，可以接受任意的服务。
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    // 获取了服务对象的描述信息  抽象层的service指针
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务的名字
    std::string service_name = pserviceDesc->name();

    // std::cout << "servicename: " << service_name << std::endl;
    LOG_INFO("servicename: %s", service_name.c_str());

    // 获取服务类对象方法的数量
    int methodCnt = pserviceDesc->method_count();

    for (int i = 0; i < methodCnt; i++)
    {
        // 获取了服务对象，指定下标的服务方法的描述   抽象的method指针。
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        // std::cout << "methodname: " << method_name << std::endl;
        LOG_INFO("methodname: %s", method_name.c_str());
    }
    service_info.m_service = service;
    // 一个服务名，对应一个服务描述，服务描述中有基类指向派生类的指针，每个服务中的方法名字和方法的对应。
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务  启动之后干什么？
void RpcProvider::Run()
{
    // 这里要获取配置文件了。InetAddress(StringArg ip, uint16_t port, bool ipv6 = false);
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");
    // 绑定链接回调和消息读写回调方法  bind的原理？
    // 为什么用到bind,因为网络层在事件发生的时候会调用onConnection和onsetMessage，但是他们是成员方法，不可以直接用名字调用，需要用bind把this指针和方法名绑定
    server.setConnectionCallback(std::bind(&RpcProvider::onConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    // 设置muduo网络库线程数量。
    server.setThreadNum(4);

    // 把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    // session timeout   30s     zkclient 网络I/O线程  1/3 * timeout 时间发送ping消息心跳消息
    ZkClient zkCli;
    zkCli.Start(); // 连接zkserver
    // service_name为永久性节点    method_name为临时性节点
    for (auto &sp : m_serviceMap) 
    {
        // /service_name   /UserServiceRpc
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0); // 创建子节点。
        for (auto &mp : sp.second.m_methodMap)
        {
            // /service_name/method_name   /UserServiceRpc/Login 存储当前这个rpc服务节点主机的ip和port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL表示znode是一个临时性节点    服务的方法设置成临时性节点，服务down了以后，保证能删除方法。
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;
    LOG_INFO("RpcProvider start service at ip: %s port: %d", ip.c_str(), port);
    // 启动网络服务
    server.start();
    m_eventLoop.loop();
}
// 新的soket连接回调
void RpcProvider::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        // 和rpc client的连接断开了
        conn->shutdown();
    }
}
/*
rpcprovider服务端的角色 和 rpcconsumer客户端的角色 要协商好通信用的protobuf数据类型
字符流应该包含service_name（调用什么服务） 和 method_name（调用什么方法） 和 args（方法的参数）   定义proto的message类型，进行数据的序列化和反序列化。否则无法区分各部分。
还需要args_size参数，因为tcp会出现数据粘包问题，所以需要长度来知道参数到哪里。

协商好的数据格式：headr_size（4B） + head_str + args_str
headr_size整数使用二进制存储。否则"1000""100"这样存储，长度不固定，也无法区分。
std::string insert 和 copy方法。适用于内存操作。
*/
// 已经连接的有读写的消息回调  如果远程有一个rpc服务的调用请求，那么onsetMessage方法就会响应。
void RpcProvider::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buffer, muduo::Timestamp tmp)
{
    // 网络上接收的远程rpc调用请求的字符流
    std::string recv_buf = buffer->retrieveAllAsString();
    // 从字符流中读取前四个字节的内容
    uint32_t header_size = 0;
    // 从recv_buf中下标为0的地方开始拷贝4个字节到以headr_size为起始的地址上。
    recv_buf.copy((char *)&header_size, 4, 0);
    // 根据header_size读取数据头的原始字符流。参数：4开始下标 headr_size长度。 包含了service_name method_name args_size
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    // 数据的反序列化
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        // 数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // 反序列化失败 记录日志
        std::cout << "rpc_header_str:" << rpc_header_str << " parse error!" << std::endl;
        return;
    }
    // 获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // 打印调试信息
    std::cout << "rpcprovider.cc" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "============================================" << std::endl;

    // 根据对方传来的字符流信息，已经解析出对方想要调用的服务，方法，参数。下面先检查他们的合法性，

    // 获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        // 没找到对应服务
        std::cout << "service_name:" << service_name << " is not exit!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << ":" << method_name << " is not exist!" << std::endl;
        return;
    }
    // 这一步是在干什么？
    google::protobuf::Service *service = it->second.m_service;      // 获取service对象 new UserService
    const google::protobuf::MethodDescriptor *method = mit->second; // 获取method对象 Login方法。

    // 生成prc方法(userservice.cc中的login)调用的请求request和response参数对象类。
    // New()构造新对象
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    // 不是已经反序列化完了吗，为什么又反序列化了?
    if (!request->ParseFromString(args_str))
    {
        std::cout << "request parse error, content: " << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this,
                                                                                                 &RpcProvider::SendRpcResponse,
                                                                                                 conn, response);

    // 调用当前rpc节点上发布的方法。
    //  new UserService().Login(contrller, request, response, done) 实际上用的基类指针操作的。
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure（userservice.cc中的）的回调操作，执行done->run()后，执行这个回调，用于序列化prc的响应和网络发送.
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str)) // response进行序列化
    {
        // 序列化成功后，通过网络把rpc方法执行的结果发送会rpc的调用方
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize response_str error!" << std::endl;
    }
    conn->shutdown(); // 模拟http的短链接服务，由rpcprovider主动断开连接
}