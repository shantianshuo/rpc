/*
    用于服务的调用方
*/
#include "mprpcchannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h> // man socket
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include "mprpcapplication.h"
#include "zookeeperutil.h"
#include "logger.h"
/*
header_size + service_name method_name args_size + args
*/
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller, // caller中的main函数传进来的，设置好错误信息之后传回去
                              const google::protobuf::Message *request,
                              google::protobuf::Message *response,
                              google::protobuf::Closure *done)
{
    // service函数：可以知道某个方法是属于什么服务的。所有通过stub代理对象调用的rpc方法，都走到这里了，统一做rpc方法调用的数据数据序列化和网络发送
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();    // service_name
    std::string method_name = method->name(); // method_name

    // 获取参数的序列化字符串长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {  
        std::cout << "debugString:" << request->DebugString() << std::endl;
        args_size = args_str.size();
    }
    else
    {
        controller->SetFailed("serialize request error!\n");
        return;
    }

    // 定义rpc的请求header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    // 序列化
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        controller->SetFailed("serialize rpcheader error!\n");
        return;
    }

    // 组织待发送的rpc请求的字符串
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4)); // header_size
    send_rpc_str += rpc_header_str;                               // rpcheader
    send_rpc_str += args_str;                                     // args

    // 打印调试信息
    std::cout << "mprpcchannel.cc" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "send_rpc_str: " << send_rpc_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "============================================" << std::endl;

    // tcp编程
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno:%d", errno);
        controller->SetFailed(errtxt); // 只能接收字符串，但需要传进去一个错误码怎么操作？见上边
        return;
    }

    // std::string rpcserverip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    // std::string rpcserverport = MprpcApplication::GetInstance().GetConfig().Load("rpcserverport");

    // rpc调用方想调用service_name的method_name服务，需要查询zk上该服务所在的host信息
    ZkClient zkCli;
    zkCli.Start();
    //  /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;
    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if (host_data == "")
    {
        controller->SetFailed(method_path + " is not exist!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return;
    }
    std::string rpcserverip = host_data.substr(0, idx);
    uint16_t rpcserverport = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    struct hostent *h; // 用于存放服务端IP的结构体。
    /*
    gethostbyname()函数为给定的主机名返回hostent类型的结构。这里的name要么是主机名，
    要么是标准点表示法的IPv4地址，要么是冒号表示法的IPv6地址（可能还有点）。
    如果name是IPv4或IPv6地址，则不执行任何查找，gethostbyname()只是将name复制到h_name字段中，
    并将其等效结构in_addr复制到返回的主机结构的h_addr_list[0]字段中。
    */
    if ((h = gethostbyname(rpcserverip.c_str())) == 0) // 把字符串格式的IP转换成结构体。
    {
        controller->SetFailed("gethostbyname failed.");
        close(clientfd);
        return;
    }
    struct sockaddr_in servaddr;            // 用于存放服务端IP和端口的结构体。
    memset(&servaddr, 0, sizeof(servaddr)); // 初始化结构体
    servaddr.sin_family = AF_INET;
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length); // 指定服务端的IP地址。
    servaddr.sin_port = htons(rpcserverport);           // 指定服务端的通信端口。

    if (-1 == connect(clientfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) // 向服务端发起连接清求。
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    // 反序列化rpc调用的响应数据
    // std::string response_str(recv_buf, 0, recv_size); // bug出现问题，recv_buf中遇到\0后面的数据就存不下来了，导致反序列化失败
    // if (!response->ParseFromString(response_str))
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        close(clientfd);
        char errtxt[1024] = {0};
        const char *format_str = "parse error! response_str:%s";
        size_t recv_buf_len = strlen(recv_buf);
        size_t format_str_len = strlen(format_str);
        if (format_str_len + recv_buf_len + 1 <= sizeof(errtxt))
        {
            sprintf(errtxt, format_str, recv_buf);
        }
        else
        {
            LOG_ERR("the recv_buf is too big!");
        }
        return;
    }

    close(clientfd);
}