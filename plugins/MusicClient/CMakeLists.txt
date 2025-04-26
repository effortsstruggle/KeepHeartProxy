

set(LIB_NAME "musicClient")

set(CMAKE_CXX_STANDARD 14) 
add_definitions(-DUSE_LOGCAT)

set( SDK_PATH "/data/T507_auto_v2.0/platform/framework/auto/sdk_lib")

include_directories( ${SDK_PATH}/include 

)

# 程序所引用的头文件库目录。
include_directories(    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。
    ${PROJECT_SOURCE_DIR}/public/cjson
    ${CMAKE_CURRENT_SOURCE_DIR}/id3v2
    "/home/ieai/workspace/Test/sky_ipc/src/"
)


file( GLOB SRC *.cpp *.c    ${PROJECT_SOURCE_DIR}/public/utils/*.cpp  ${CMAKE_CURRENT_SOURCE_DIR}/id3v2/*.cpp)


# message("MusicClient src ${SRC}")
# 设置动态库生成目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})

#message("aaaaaaa ${PACK_PLUGINS_DIR}")
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





