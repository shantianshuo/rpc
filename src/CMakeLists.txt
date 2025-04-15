# aux_source_directory(. SRC_LIST) # 当前目录的所有文件，放入SRC_LIST变量中。 尽量用set，否则新增加文件无法识别。
set(SRC_LIST 
    mprpcapplication.cc 
    mprpcconfig.cc 
    rpcprovider.cc 
    rpcheader.pb.cc 
    mprpcchannel.cc 
    mprpccontroller.cc
    logger.cc
    zookeeperutil.cc)
# mprpc：生成的动态库名字   SHARED：表明生成的是动态库 SRC_LIST：源文件路径
# 后续：改为了静态库，因为muduo是静态，静态动态一起编译，会出问题。
add_library(mprpc  ${SRC_LIST}) # 生成动态库
# 啥时候会用到这个呢？ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/user/tools 设置环境变量
# 框架用了muduo 顺序不能乱
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)