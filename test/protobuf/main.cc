#include "test.pb.h"
#include <iostream>
#include <string>
using namespace fixbug;

int main()
{
    /*
    LoginResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(1);
    rc->set_errmsg("longin error!");
    */
    GetFriendListsResponse rsp;
    // 对象里边还是对象怎么操作？
    ResultCode *rc = rsp.mutable_result(); // mutable返回对象可修改。
    rc->set_errcode(0);
    // 列表操作
    User *user1 = rsp.add_friend_list(); // add_表示往朋友列表里边加元素
    user1->set_age(20);
    user1->set_name("zhangsan");
    user1->set_sex(User::MAN); // 枚举类型用法

    User *user2 = rsp.add_friend_list();
    user2->set_age(22);
    user2->set_name("lisi");
    user2->set_sex(User::MAN);

    std::cout << rsp.friend_list_size() << std::endl; // 获取成员个数

    // 读取列表数据
    User user = rsp.friend_list(1);
    std::string name = user.name();
    int age = user.age();
    User_Sex sex = user.sex();
    std::cout << name << " " << age << " " << sex << std::endl;
    return 0;
}

#if 0
int main1()
{
    //封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zhangsan");
    req.set_pwd("123456");
    //对象数据序列化
    std::string send_str;
    if (req.SerializePartialToString(&send_str))
    {
        // true代表序列化成功。
        std::cout << send_str.c_str() << std::endl;
    }

    //字符串反序列化成login请求对象。
    LoginRequest reqB;
    if(reqB.LoginRequest::ParseFromString(send_str))
    {
        //true代表反序列化成功
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }
    return 0;
}
#endif