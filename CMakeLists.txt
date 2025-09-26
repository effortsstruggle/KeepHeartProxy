cmake_minimum_required(VERSION 3.10)


project(KeepHeardProxy VERSION 0.1.0 LANGUAGES C CXX)

# Esc[0;31m
set(R "${Esc}[0;31m")   #红色
set(E "${Esc}[m" )      #结束颜色设置
set(B "${Esc}[1;34m")   #蓝色高亮
set(Y "${Esc}[1;33m")   #黄色高亮 
set(RB "${Esc}[1;31;40m") #红色字体黑色背景
string(ASCII 27 Esc)

set(CMAKE_CXX_STANDARD 11)
#编译参数
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -O1 -s -march=native")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -Wall -O1 -s")
# 全局的宏定义
set(PUSH_REMOTE 0)  #   1 0 # 是否推送到远程，不管启动与否都会推送到打包目录
set(COPY_TO 0)  # 是否拷贝到打包目录
set(BUILD_FOR "LGNJ01")   # LGNJ01  # GLBM01 设置项目编号
set(CMAKE_BUILD_TYPE "Release") #  Debug   Release 构建类型

set(PACK_LIBS_DIR "/home/qin/workspace/KeepHeartProxyDependsLib")   # 打包库目录
set(PACK_PLUGINS_DIR "/home/qin/workspace/KeepHeartProxyDependsLib/plugins") # 插件打包目录
set(PACK_PUBLIC_DIR "/home/qin/workspace/KeepHeartProxyDependsLib/public") # 插件打包目录
set(PACK_TEST_DIR "/home/qin/workspace/KeepHeartProxyDependsLib/test") # 插件打包目录



add_definitions(-DUSE_LOGCAT)

function(color_out  c str)

    # message("para ${c} ,${str}")
    if (${c} EQUAL  1)
        message("${B}${str}${E}")
    elseif (${c} EQUAL  2)
        message("${R}${str}${E}")
    elseif (${c} EQUAL  3)
        message("${Y}${str}${E}")

    else()
    message("${str}")
    endif()
endfunction()



# 指定编译目标平台，一般是Linux，Windows Generic t5.cmake
include(cmake_modules/AutoGenVer.cmake)


#打印构建信息
color_out(3 "build for ${BUILD_FOR}; \nbuild type ${CMAKE_BUILD_TYPE}; \npush_remote ${PUSH_REMOTE} \ncopy to package ${COPY_TO} ")

include_directories (
    "${PROJECT_SOURCE_DIR}/public/utils/"
    "${PROJECT_SOURCE_DIR}/public/pluginsManger/"
)

# 不需要编译的就注释，不参与编译
add_subdirectory(public)
add_subdirectory(proxy)
add_subdirectory(plugins)
add_subdirectory(test)