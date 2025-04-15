#pragma once
#include <queue>
#include <thread>
#include <mutex> // pthread_mutex_t
#include <condition_variable> // pthread_condition_t

// 异步写日志的日志队列
template<typename T>
class LockQueue
{
public:
    // 多个worker线程都会写日志queue 
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one(); // 只有一个线程在写，one就行。
    }

    // 一个线程读日志queue，写日志文件
    T Pop() // 这里不能用T& 否则返回局部变量引用会出问题
    {
        // 条件变量需要unique_lock这个锁，要不然lock_guard不能作为参数传递。
        std::unique_lock<std::mutex> lock(m_mutex);
        // 线程的通信
        while (m_queue.empty())
        {
            // 日志队列为空，线程进入wait状态
            m_condvariable.wait(lock); // 等待且释放锁
        }

        T data = m_queue.front(); // 获取队头元素
        m_queue.pop();
        return data;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
};