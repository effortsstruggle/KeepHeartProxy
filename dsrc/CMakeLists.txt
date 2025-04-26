set(LIB_NAME "SkyProxy")
#set(CMAKE_BUILD_TYPE "Debug")
# 或
set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH "/home/qin/qt5.15.2/")
find_package(QT NAMES  Qt5 REQUIRED COMPONENTS  Core Qml Quick )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Qml Quick )

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PROJECT_SOURCES ${SRC} )

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dsrc)

set(DLL_DIR "/home/ieai/workspace/greenWorks/Qt_BLL/libs/")
set(HEADER_DIR "/home/ieai/workspace/greenWorks/Qt_BLL/include/")


# # 头文件
include_directories ("./" 
${HEADER_DIR}
${PROJECT_SOURCE_DIR}/pluginsManger
)

file(GLOB_RECURSE SRC  ./*.cpp ./*.c )

# 库目录
link_directories(${DLL_DIR})


# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})
# else()
#     add_executable(${LIB_NAME} main.cpp )
#     target_link_libraries(${LIB_NAME} "${PROJECT_SOURCE_DIR}/build/libSkyProxy.so")
# endif()


target_link_libraries(${LIB_NAME} Qt5::Core  Qt5::Qml Qt5::Quick)
target_link_libraries(${LIB_NAME} -lpthread -lcjson )
set (SKYPROXY_SO "${CMAKE_BINARY_DIR}/dsrc/lib${LIB_NAME}.so")

if(PUSH_REMOTE)
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SKYPROXY_SO} ${DLL_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${SKYPROXY_SO} ${PACK_LIBS_DIR}

    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/../dsrc/SkyProxy.h ${HEADER_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/../dsrc/qt_bll_proxy.h ${HEADER_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/../dsrc/qt_dvrapi_proxy.h ${HEADER_DIR}
    # COMMAND  scp ${CMAKE_BINARY_DIR}/lib${LIB_NAME}.so root@192.168.10.116:/apps/libs/
    COMMAND  scp ${SKYPROXY_SO} root@192.168.49.1:/apps/libs/

    COMMAND echo  lib${LIB_NAME}.so push remote success
  )
else()
    add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${SKYPROXY_SO} ${PACK_LIBS_DIR}
    COMMAND echo cp to pack success
  )
endif()


