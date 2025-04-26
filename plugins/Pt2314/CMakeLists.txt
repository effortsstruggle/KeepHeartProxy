
cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 11)

# Declares and names the project.


set(CMAKE_FIND_ROOT_PATH /opt/t5sdk/aarch64-buildroot-linux-gnu/sysroot)



set( SDK_PATH "/data/T507_auto_v2.0/platform/framework/auto/sdk_lib")
set(CDXINC ${SDK_PATH}/cedarx/include)
set(CDXEXT ${SDK_PATH}/cedarx/include/external/include)
set(CDXCOR ${SDK_PATH}/cedarx/include/libcore)
set(CDXMETA ${SDK_PATH}/xmetadataretriever/)

set(LIB_NAME "pt2313")
message("ENV ${CPLUS_INCLUDE_PATH}")
include_directories(   
        ${SDK_PATH}/include/linux
        ${SDK_PATH}/include 
        ${SDK_PATH}/include/utils 
        ${SDK_PATH}/include/sound/ 
        ${SDK_PATH}/include/memory/ 
        ${SDK_PATH}/sdk_misc/ 
        ${SDK_PATH}/include/audioenc
        ${SDK_PATH}/cedarx/include/libcedarc/include
        ${PROJECT_SOURCE_DIR}
)



add_definitions(-DUSE_LOGCAT)

link_directories(${SDK_PATH}/cedarx/lib/)
link_directories( ${SDK_PATH}/lib64)

aux_source_directory(./ SRC_LIST)

message("SRC_LIST:${SRC_LIST} " )


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

add_library(${LIB_NAME}  SHARED  ${SRC_LIST})

target_link_libraries(${LIB_NAME}  -lsdk_log   -lsdk_storage    -lpthread )



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

