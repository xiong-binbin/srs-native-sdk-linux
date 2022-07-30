cmake_minimum_required (VERSION 3.8)

#项目名称
project(srs-native-sdk)
set(PROJECT srs-native-sdk)

#使用c++11标准
add_definitions(-std=c++11)

#配置Debug或Release版本
set(CMAKE_BUILD_TYPE "Debug")
# set(CMAKE_BUILD_TYPE "Release")

#引入头文件
include_directories(
        ./include
        /usr/local/include
)

#引入源文件
file(GLOB SRC_LIST "src/*.cpp" "main.cpp")

#生成可执行文件
add_executable(${PROJECT} ${SRC_LIST})

#生成动态库
#add_library(${PROJECT} SHARED ${SRC_LIST})

#生成静态库
#add_library(${PROJECT} STATIC ${SRC_LIST})

#链接静态库和动态库
target_link_libraries(
        ${PROJECT}
        -Wl,--start-group
        /usr/local/lib/libwebrtc.a
        -Wl,--end-group
)