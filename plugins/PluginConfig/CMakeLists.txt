

set(LIB_NAME "PluginConfig")
set(LIB_DIR "PluginConfig")

add_definitions(-DUSE_LOGCAT)


# 程序所引用的头文件库目录。
include_directories(    
    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。
    ${PROJECT_SOURCE_DIR}/public/cjson
    ${PROJECT_SOURCE_DIR}/public/utils
)

link_directories(
   ${CMAKE_BINARY_DIR}/public/utils 
   ${CMAKE_BINARY_DIR}/public/cjson
)

file( GLOB SRC *.cpp *.c )

# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})
target_link_libraries(${LIB_NAME} -lpthread -lcjson -lutils)

push_to_target()