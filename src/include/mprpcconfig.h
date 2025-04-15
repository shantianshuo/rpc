/*
    封装用于配置文件的读取类。
        成员：m_configMap
        1.LoadConfigFile函数：传入配置文件名字字符串config_file，解析配置文件信息，保存到m_configMap中。
          MprpcApplication的Init函数会用到
        2.Load函数：可以查询m_configMap。
          RpcProvider的Run方法会用到。
*/
#pragma once

#include <unordered_map> // 配置文件需要键值对，但是又不用排序。
#include <string> //键值都是string类型

class MprpcConfig
{
public:
    // 为什么要有两个加载函数？都啥时候调用了

    // 负责解析加载配置文件
    void LoadConfigFile(const char *config_file);
    // 查询配置项信息 在m_configMap通过健来查询
    std::string Load(const std::string key);
private:
    std::unordered_map<std::string, std::string> m_configMap;
    // 去掉字符串前后的空格  LoadConfigFile()中会用
    void Trim(std::string &src_buf);
};

