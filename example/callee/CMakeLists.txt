set(SRC_LIST userservice.cc ../user.pb.cc)
# set(SRC_LIST friendservice.cc ../friend.pb.cc)
# provider:生成的可执行文件名字 SRC_LIST：源文件的集合
add_executable(provider ${SRC_LIST})
# callee需要使用框架源码，所以要链接rpc的动态库。也需要使用protobuf的功能，所以也要链接protobuf的动态库
# provider:动态库的使用者  mprpc和protobuf：动态库所在的地方。
target_link_libraries(provider mprpc protobuf)
