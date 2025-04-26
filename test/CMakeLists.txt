
cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_BUILD_TYPE "Debug") # Release   Debug

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_PREFIX_PATH "/opt/T5_Qt_5.15.8_Release/")
find_package(QT NAMES  Qt5 REQUIRED COMPONENTS  Core Qml Quick )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Qml Quick )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
include(/opt/t5sdk/t5sdk.cmake)
# Declares and names the project.
set(DLL_DIR "/home/ieai/workspace/greenWorks/Qt_BLL/libs/")
set(HEADER_DIR "/home/ieai/workspace/greenWorks/Qt_BLL/include/")


# # 头文件
include_directories ("./" 
${HEADER_DIR}
${PROJECT_SOURCE_DIR}/pluginsManger
)


link_directories(${PACK_LIBS_DIR})
link_directories(${DLL_DIR})

set(LIB_NAME "test_skyproxy")



# 程序所引用的头文件库目录。
include_directories(    ${CMAKE_CURRENT_SOURCE_DIR}	# 当前工程的文件路径。如果需要其他库，要继续写所需要库的路径。
  ${PROJECT_SOURCE_DIR/dsrc}
)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test)

add_definitions(-DUSE_LOGCAT)


aux_source_directory(. SRC_LIST)

message("SRC_LIST:"  ${SRC_LIST})

add_executable(${LIB_NAME}   ${SRC_LIST})

target_link_libraries(${LIB_NAME} Qt5::Core  Qt5::Qml Qt5::Quick)
target_link_libraries(${LIB_NAME}  -lSkyProxy  -lsky_utils -lmcu_parser_YPS -lyps_bll    -lm  -lpthread  -llog -lcarbitUtil -lsqlite3 pthread -lsky_ipc -lmcu -lcjson "libdvrapi.a" "libmplayerapi.a" )


if( ${PUSH_REMOTE} EQUAL 1)
add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
    COMMAND  scp ${CMAKE_BINARY_DIR}/test/${LIB_NAME} root@192.168.49.1:/apps
  
    COMMAND echo "${LIB_NAME}" push remote success
)
endif()





