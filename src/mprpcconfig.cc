/*
    封装用于配置文件的读取类
*/
#include "mprpcconfig.h"
#include <iostream> // include什么时候在头文件写？什么时候在源文件写？
#include <string> // buf字符串转为string类型，好操作。

// 负责解析加载配置文件     传入的参数是配置文件字符串
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    // 打开文件    "r"：用读的方式打开的。 
    FILE *pf = fopen(config_file, "r");
    if(pf == nullptr)
    {
        std::cout << config_file << "is not exist!" << std::endl;
        exit(EXIT_FAILURE); // 配置文件都不对，没必要进行了。
    }

    // 读取是一行一行读取  feof()的原理？
    // 处理的情况：1.注释 2.正确的配置项 3.去掉开头的多余的空格
    while(!feof(pf))
    {
        char buf[512] = {0}; // 同char buf[512] = {'\0'};
        fgets(buf, 512, pf); // 读512个，如果一行没有512会读好多的\0，还是根据实际的来呢？

        // char[] -> std::string
        std::string read_buf(buf);
        Trim(read_buf);

        // 经过处理，如果有#一定在开头。
        // 判断注释
        if(read_buf[0] == '#' || read_buf.empty()) // read_buf.empty()：这一行都是空格
        {
            continue; // 读下一行吧。
        }

        int idx = read_buf.find('=');
        if(idx == -1)
        {
            //配置项不合法
            continue;
        }
        //正确的配置项，解析
        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        Trim(key); // 防止key后边有空格
        int endidx = read_buf.find('\n', idx);
        value = read_buf.substr(idx + 1, endidx - idx - 1);
        Trim(value);
        m_configMap.insert({key, value});
    }
}
// 查询配置项信息 根据健查询值。
std::string MprpcConfig::Load(const std::string key)
{
    // return m_configMap[key]; 一定不用中括号，因为key不存在会增加东西。
    auto it = m_configMap.find(key);
    if(it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}

// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' '); // 找到第一个不是空格的。找不到返回 -1
    if(idx != -1)
    {
        // 前边有空格（如果后边有空格怎么办？） 参数：起始下标，长度
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    //去掉后边的空格。
    idx = src_buf.find_last_not_of(' ');
    if(idx != -1)
    {
        src_buf = src_buf.substr(0, idx + 1);
    }
}