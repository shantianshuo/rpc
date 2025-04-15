cmake_minimum_required (VERSION 3.0)
project (mprpc)

# 可以进行调试。
set(CMAKE_BUILD_TYPE "Debug")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) #可执行文件输出路径
set (LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib) #库文件输出路径

include_directories(${PROJECT_SOURCE_DIR}/src/include) # 头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/example)
link_directories(${PROJECT_SOURCE_DIR}/lib) # 库文件搜索路径 -L

add_subdirectory(src)#加载子目录 目标目录里也应该有CMakeLists.txt
add_subdirectory(example)