

set(LIB_NAME "glbErrCodeProxy")

# set( SDK_PATH "/data/T507_auto_v2.0/platform/framework/auto/sdk_lib")
# include_directories( ${SDK_PATH}/include )

add_definitions(-DUSE_LOGCAT)


# 程序所引用的头文件库目录。
include_directories(    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。
    ${PROJECT_SOURCE_DIR}/public/cjson
    ${PROJECT_SOURCE_DIR}/public/utils
)


file( GLOB SRC *.cpp *.c )

message("aaa ${SRC}")
# 设置动态库生成目录
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})


target_link_libraries(${LIB_NAME} -lcjson -lsky_utils)



push_to_target()




