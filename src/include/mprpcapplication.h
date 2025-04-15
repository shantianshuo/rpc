/*
    框架的初始化
        成员：m_config
        使用单例模式构建，因为配置文件只需要读取一次，初始化也只需要进行一次。
        1.Init函数：传入命令行参数，判断命令格式是否合法，将配置文件名存储在config_file中。
            调用LoadConfigFile方法，将config_file中的文件读取到成员m_config对象的m_configMap成员中。
        2.GetInstance函数：获取MprpcApplication类的唯一对象app。
        3.GetConfig函数：获取app中对象中的唯一对象m_config。
*/
#pragma once // 防止头文件重复包含。

#include "mprpcconfig.h"
#include "mprpccontroller.h" // 如果用到mprpcapplication.h的时候都用到这两个，可以优化到这个头文件中。
#include "mprpcchannel.h"

// 框架的基础类，负责框架的一些初始化操作。
class MprpcApplication
{
public:
    // 方法为什么使用静态的方法呢？
    static void Init(int argc, char **argv);
    static MprpcApplication &GetInstance();
    static MprpcConfig& GetConfig();

private:
    static MprpcConfig m_config; // 因为要在Init函数中访问，静态成员函数不能访问普通的成员变量,所以用static修饰

    MprpcApplication(){};
    MprpcApplication(const MprpcApplication &) = delete;
    MprpcApplication(MprpcApplication &&) = delete;
};