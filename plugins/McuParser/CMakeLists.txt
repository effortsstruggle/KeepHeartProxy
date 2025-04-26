

set(LIB_NAME "mcuPraser")

set( SDK_PATH "/data/T507_auto_v2.0/platform/framework/auto/sdk_lib")



include_directories( ${SDK_PATH}/include 

)

add_definitions(-DUSE_LOGCAT)


# 程序所引用的头文件库目录。
include_directories(    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。
    ${PROJECT_SOURCE_DIR}/public/mcu_comm_lib
    ${PROJECT_SOURCE_DIR}/public/cjson
)


file( GLOB SRC *.cpp  )

# message("aaa ${SRC}")
# 设置动态库生成目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})


target_link_libraries(${LIB_NAME} -lcjson)



if( ${PUSH_REMOTE} EQUAL 1)
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND  scp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so root@192.168.49.1:/apps/plugins/
    COMMAND  cp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so  ${PACK_PLUGINS_DIR}
    COMMAND echo "${LIB_NAME}" push remote success
  )
else()
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND  cp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so  ${PACK_PLUGINS_DIR}
    COMMAND  echo "${LIB_NAME}" cp to pack  success
  )
endif()




