

set(LIB_NAME "mplayerClient")


add_definitions(-DUSE_LOGCAT)

set( SDK_PATH "/data/T507_auto_v2.0/platform/framework/auto/sdk_lib")
include_directories( ${SDK_PATH}/include 
)
# 程序所引用的头文件库目录。
include_directories(    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。
                        ${PROJECT_SOURCE_DIR}/public/cjson
)


file( GLOB SRC *.cpp *.c    ${PROJECT_SOURCE_DIR}/public/utils/*.cpp )


# message("MusicClient src ${SRC}")
# 设置动态库生成目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})

target_link_libraries(${LIB_NAME} -llog -lcarbitUtil -lsqlite3 pthread  -lmcu -lcjson "libdvrapi.a" "libmplayerapi.a")

add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
 
  COMMAND  scp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so root@192.168.49.1:/apps/plugins/
#  COMMAND  adb push ${CMAKE_BINARY_DIR}/lib${LIB_NAME}.so /apps/libs

  COMMAND echo "${LIB_NAME}" push success
)





