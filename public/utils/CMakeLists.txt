set(LIB_NAME "utils")
set(LIB_DIR "utils")

#编译参数
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -O2 -s")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -Wall -O2 -s")

file(GLOB SRC *.cpp *.c)

# 程序所引用的头文件库目录。
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。

)

# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})
target_link_libraries(${LIB_NAME}   -lpthread  )

push_to_target()