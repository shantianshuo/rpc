/*
    框架的初始化
*/

#include "mprpcapplication.h"
#include <iostream> //打印日志信息
#include <unistd.h> // getopt()函数
#include <string> // 定义config_file

MprpcConfig MprpcApplication::m_config; // 在类外初始化成员

void ShowArgsHelp()
{
    std::cout << "format: command -i <configfile>" << std::endl;
}
// 读取配置文件，日志的设置。
/*
char **argv 一个*和两个*的argv有什么区别？
*/
void MprpcApplication::Init(int argc, char **argv)
{
    // 未传入任何参数，不允许的操作。
    if(argc < 2)
    {
        ShowArgsHelp();
        // 程序以失败状态退出  在终止之前，它会执行一些清理工作
        exit(EXIT_FAILURE);
    }
    int c = 0;
    std::string config_file;
    // getopt()的原理？
    while((c = getopt(argc, argv, "i:")) != -1)
    {
        switch (c)
        {
        case 'i': // 有配置文件了 
            // optarg: unistd头文件的全局变量。  optarg变量有什么作用？
            config_file = optarg;
            break;
        case '?': // 出现了不想要的参数
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':': // 出现了-i但是没有参数
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break; //break情况 和exit情况有什么不同呢？
        }
    }
    if(config_file.empty())
    {
        std::cout << "-i is needed" << std::endl;
        exit(EXIT_FAILURE);
    }
    // 已经正确输入了command -i <configfile>命令。开始加载配置文件。
    /*
    rpcserver_ip=  rpcserver_port   zookeeper_ip=  zookepper_port=
    */
    // 模块化思想：加载文件操作应该提出去。
    /*
    新增.h .cc文件，构建类，成员方法。在本类中添加成员变量m_config，调用成员函数。
    1.怎么封装的？ 2.返回值如何带回来？ 3.配置文件怎么处理成的字符串了？
    */
    m_config.LoadConfigFile(config_file.c_str());

    // 测试
    // std::cout << "rpcserverip:" << m_config.Load("rpcserverip") << std::endl;
    // std::cout << "rpcserverport:" << m_config.Load("rpcserverport") << std::endl;
    // std::cout << "zookeeperip:" << m_config.Load("zookeeperip") << std::endl;
    // std::cout << "zookeeperport:" << m_config.Load("zookeeperport") << std::endl;

}

// 这个方法很重要。
MprpcApplication &MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}