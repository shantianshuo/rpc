#pragma once
#include <google/protobuf/service.h>
#include <string>

class MprpcController : public google::protobuf::RpcController
{
public:
    MprpcController();
    void Reset(); // 成员变量置为初始状态
    bool Failed() const; // 调用过程成功与否
    std::string ErrorText() const; // 返回错误信息
    void SetFailed(const std::string& reason); // 设置错误。

    // 目前未实现具体的功能
    void StartCancel();
    bool IsCanceled() const;
    void NotifyOnCancel(google::protobuf::Closure* callback);
private:
    bool m_failed; // RPC方法执行过程中的状态
    std::string m_errText; // RPC方法执行过程中的错误信息
};