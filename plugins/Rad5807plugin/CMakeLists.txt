
set(LIB_NAME "FM5807")

# 程序所引用的头文件库目录。
include_directories(
    ${PROJECT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。
    # ../../dsrc
)


file(GLOB SRC *.cpp *.c)



# 设置动态库生成目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})






