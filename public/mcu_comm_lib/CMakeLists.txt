cmake_minimum_required(VERSION 3.0.0)


set(CMAKE_SYSTEM_NAME Linux)
set(PUSH_WAY 1)
# 指定编译器绝对路径。其它工具，cmake会自动推导
set(CMAKE_C_COMPILER /opt/t5sdk/toolchain/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /opt/t5sdk/toolchain/bin/aarch64-linux-gnu-g++)

# 指定目标系统根目录，lib，include 等查找，会自动从根目录开始
set(CMAKE_FIND_ROOT_PATH /opt/t5sdk/aarch64-buildroot-linux-gnu/sysroot)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

project(mcu VERSION 0.1.0 LANGUAGES C CXX)

set( SDK_PATH "/data/T507_auto_v2.0/platform/framework/auto/sdk_lib")

include_directories( ${SDK_PATH}/include )

add_definitions(-DUSE_LOGCAT)


aux_source_directory(. SRC)

set(LIB_NAME "mcu")

include_directories (${CMAKE_INCLUDE_CURRENT_DIR})


message("SRC: ${SRC}")


# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})




set(TARGET_LIB "/home/ieai/workspace/greenWorks/Qt_BLL/libs")
set(TARGET_INC "/home/ieai/workspace/greenWorks//Qt_BLL/include")
set(PACK_DIR "/home/ieai/workspace/updateMake/user-resource/GLBM01/libs/")
if( ${PUSH_WAY} EQUAL 1)
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib${LIB_NAME}.so ${TARGET_LIB}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib${LIB_NAME}.so ${PACK_DIR}
    COMMAND  scp ${CMAKE_BINARY_DIR}/lib${LIB_NAME}.so root@192.168.49.1:/apps/libs/
    COMMAND echo push remote lib${LIB_NAME}.so success
  )
else()
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib${LIB_NAME}.so ${TARGET_LIB}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib${LIB_NAME}.so ${PACK_DIR}

    COMMAND echo push lib${LIB_NAME}.so success
  )
endif()




