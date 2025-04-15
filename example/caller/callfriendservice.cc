#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"
#include "logger.h"

int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);
    fixbug::FiendServiceRpc_Stub stub(new MprpcChannel());
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);
    fixbug::GetFriendsListResponse response;
    MprpcController controller;
    // controller 为nullptr时，默认一定会成功。
    stub.GetFriendsList(&controller, &request, &response, nullptr);

    if(controller.Failed())
    {
        // rpc调用过程中出现了错误，比如序列化反序列化，网络发送，接收等环节。
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        // 判断对端的函数调用是否成功
        if (0 == response.result().errcode())
        {
            std::cout << "rpc login GetFriendsList success!" << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; i++)
            {
                std::cout << "index:" << (i + 1) << "name:" << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc login GetFriendsList error : " << response.result().errmsg() << std::endl;
        }
    }
    
    return 0;
}
