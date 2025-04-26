


# set(CMAKE_BUILD_TYPE Debug)
include("/opt/t5sdk/t5sdk.cmake")


set(LIB_NAME "sky_utils")
# 程序所引用的头文件库目录。
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。

)

 
#编译参数
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -O2 -s")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -g -Wall -O2 -s")


file(GLOB SRC *.cpp *.c)

# aux_source_directory(${PROJECT_SOURCE_DIR}/public/pluginsManger SRC) 


# 设置动态库生成目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/public)

# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})

# add_executable("${LIB_NAME}"  ${SRC})

target_link_libraries(${LIB_NAME}   -lpthread  )


if( ${PUSH_REMOTE} EQUAL 1)
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND  scp ${CMAKE_BINARY_DIR}/public/lib${LIB_NAME}.so root@192.168.49.1:/apps/libs/
    COMMAND  cp ${CMAKE_BINARY_DIR}/public/lib${LIB_NAME}.so  ${PACK_LIBS_DIR}
    COMMAND echo "${LIB_NAME}" push remote success
  )
else()
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND  cp ${CMAKE_BINARY_DIR}/public/lib${LIB_NAME}.so  ${PACK_LIBS_DIR}
    COMMAND echo "${LIB_NAME}" cp to pack  success
  )
endif()