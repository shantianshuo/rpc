// 框架提供的日志系统。
// 写日志是一个磁盘io，需要在框架和log之间添加中间层，同步通信和互斥。
#pragma once
#include "lockqueue.h"
#include <string>

// 定义宏 LOG_INFO("xxx %d %s", 20, "xxxx")
// 可变参数的LOG_INFO  logmsgformat：格式化字符串
#define LOG_INFO(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(INFO); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.WriteToLogQueue(c); \
    } while(0) \

#define LOG_ERR(logmsgformat, ...) \
    do \
    {  \
        Logger &logger = Logger::GetInstance(); \
        logger.SetLogLevel(ERROR); \
        char c[1024] = {0}; \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.WriteToLogQueue(c); \
    } while(0) \

// 定义日志级别
enum LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
};

// Mprpc框架提供的日志系统
class Logger
{
public:
    // 获取日志的单例
    static Logger& GetInstance();
    // 设置日志级别 
    void SetLogLevel(LogLevel level);
    // 写日志
    void WriteToLogQueue(std::string msg);
private:
    int m_loglevel; // 记录日志级别 正常还是错误？
    LockQueue<std::string>  m_lckQue; // 日志缓冲队列

    Logger();
    Logger(const Logger&) = delete; // 删除拷贝构造
    Logger(Logger&&) = delete;
};
