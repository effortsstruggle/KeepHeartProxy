


#set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_FIND_ROOT_PATH /opt/t5sdk/aarch64-buildroot-linux-gnu/sysroot)

set( SDK_PATH "/data/T507_auto_v2.0/platform/framework/auto/sdk_lib")

set(CDXINC ${SDK_PATH}/cedarx/include)
set(CDXEXT ${SDK_PATH}/cedarx/include/external/include)
set(CDXCOR ${SDK_PATH}/cedarx/include/libcore)
set(CDXMETA ${SDK_PATH}/xmetadataretriever/)

include_directories(   
        ${SDK_PATH}/include/linux
        ${SDK_PATH}/include 
        ${SDK_PATH}/include/disp2 	
        ${SDK_PATH}/include/utils 
        ${SDK_PATH}/include/cutils/ 
        ${SDK_PATH}/include/storage
        ${SDK_PATH}/include/sound/ 
        ${SDK_PATH}/include/memory/ 
        ${SDK_PATH}/sdk_misc/ 
        ${SDK_PATH}/include/audioenc
        ${SDK_PATH}/cedarx/include/libcedarc/include

)


# 宏定义
add_definitions(-DUSE_LOGCAT)

if(${BUILD_FOR} STREQUAL "LGNJ01")
  message("build si475x for LGNJ01 ")
  add_definitions(-DLGNJ01)
endif()


link_directories(${SDK_PATH}/cedarx/lib/)
link_directories( ${SDK_PATH}/lib64)



set(LIB_NAME "FM_Si475")

# 程序所引用的头文件库目录。
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。
    ${PROJECT_SOURCE_DIR}/dsrc
    ${PROJECT_SOURCE_DIR}/public/cjson
    ${PROJECT_SOURCE_DIR}/public/pluginsManger
    ${PROJECT_SOURCE_DIR}/public/utils
)


file(GLOB SRC *.cpp *.c)


# 设置动态库生成目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})


target_link_libraries(${LIB_NAME}  -lsdk_log   -lsdk_storage    -lpthread -lsky_utils )


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