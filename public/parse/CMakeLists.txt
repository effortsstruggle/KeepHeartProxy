cmake_minimum_required(VERSION 3.0.0)

set(LIB_NAME "parse")
set(LIB_DIR "parse")

add_definitions(-DUSE_LOGCAT)

file(GLOB SRC *.cpp *.c)

include_directories (
    ${CMAKE_INCLUDE_CURRENT_DIR}
    ${PROJECT_SOURCE_DIR}/public/utils
    )

# lib search path
link_directories( ${CMAKE_BINARY_DIR}/public/utils )

# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})
target_link_libraries(${LIB_NAME} -lutils)

push_to_target()



