
cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 11)

# Declares and names the project.

# #设置编译工具链
set(TOOLCHAIN_DIR "/opt/t5sdk/toolchain/bin")
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/aarch64-linux-gnu-gcc) 
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/aarch64-linux-gnu-g++)
set(ARM_SYSROOT /opt/t5sdk/aarch64-buildroot-linux-gnu/sysroot)

#指定交叉编译环境的目录
set(CMAKE_FIND_ROOT_PATH "${ARM_SYSROOT}")
#从来不在指定目录(交叉编译)下查找工具程序。(编译时利用的是宿主的工具)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#只在指定目录(交叉编译)下查找库文件
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#只在指定目录(交叉编译)下查找头文件
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#只在指定的目录(交叉编译)下查找依赖包
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_CXX_FLAGS "-g -Wall  --sysroot=${ARM_SYSROOT}")
set(CMAKE_CXX_LINK_FLAGS "--sysroot=${ARM_SYSROOT}")
set(CMAKE_C_FLAGS "-g -Wall --sysroot=${ARM_SYSROOT}")
set(CMAKE_C_LINK_FLAGS "--sysroot=${ARM_SYSROOT}")
#设置编译的版本为debug版本。如果要编译realse版本，直接写realse即可
set(CMAKE_BUILD_TYPE Debug)


set(LIB_NAME "ICM42680")

set( SDK_PATH "/data/T507_auto_v2.0/platform/framework/auto/sdk_lib")

# /home/gh/T507_auto_v2.0/platform/framework/libcedarc/include
set(CDXINC ${SDK_PATH}/cedarx/include)
set(CDXEXT ${SDK_PATH}/cedarx/include/external/include)
set(CDXCOR ${SDK_PATH}/cedarx/include/libcore)
set(CDXMETA ${SDK_PATH}/xmetadataretriever/)


message("ENV ${CPLUS_INCLUDE_PATH}")
include_directories(   
        ${SDK_PATH}/include/linux
        ${SDK_PATH}/include 
        ${SDK_PATH}/include/disp2 	
        ${SDK_PATH}/include/utils 
        ${SDK_PATH}/include/cutils/ 
        ${SDK_PATH}/include/media 
        ${SDK_PATH}/include/storage
        ${SDK_PATH}/include/sound/ 
        ${SDK_PATH}/include/memory/ 
        ${SDK_PATH}/sdk_misc/ 
        ${SDK_PATH}/include/audioenc
        ${SDK_PATH}/cedarx/include/libcedarc/include

)

# 程序所引用的头文件库目录。
include_directories(    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。
    ${PROJECT_SOURCE_DIR}/public/cjson
)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

add_definitions(-DUSE_LOGCAT)

link_directories(${SDK_PATH}/cedarx/lib/)
link_directories( ${SDK_PATH}/lib64)

aux_source_directory(. SRC_LIST)

message("SRC_LIST:"  ${SRC_LIST})

add_library(${LIB_NAME}  SHARED  ${SRC_LIST})

target_link_libraries(${LIB_NAME}  -lsdk_log   -lsdk_storage   -lm  -lpthread -lz  -lssl )



if( ${PUSH_REMOTE} EQUAL 1)
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND  scp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so root@192.168.49.1:/apps/plugins/
    COMMAND  cp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so  ${PACK_PLUGINS_DIR}
    COMMAND echo "${LIB_NAME}" push remote success
  )
else()
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND  cp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so  ${PACK_PLUGINS_DIR}
    COMMAND echo "${LIB_NAME}" cp to pack  success
  )
endif()




