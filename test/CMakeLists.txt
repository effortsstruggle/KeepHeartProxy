cmake_minimum_required(VERSION 3.0.0)

set(LIB_NAME "TestKeepHeartProxy")

set(CMAKE_BUILD_TYPE "Debug") # Release   Debug

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(CMAKE_CXX_STANDARD 11)

set(CMAKE_PREFIX_PATH "/home/qin/qt5.15.2/")
find_package(QT NAMES  Qt5 REQUIRED COMPONENTS  Core Qml Quick )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Qml Quick )

add_definitions(-DUSE_LOGCAT)

# 头文件
include_directories (
  ${PROJECT_SOURCE_DIR}/dsrc
  ${PROJECT_SOURCE_DIR}/public/utils
  ${PROJECT_SOURCE_DIR}/public/cjson
)

file( GLOB SRC *.cpp *.c )

link_directories( 
  ${CMAKE_BINARY_DIR}/dsrc
  ${CMAKE_BINARY_DIR}/public/utils
  ${CMAKE_BINARY_DIR}/public/cjson
)

add_executable( ${LIB_NAME} ${SRC} )
target_link_libraries(${LIB_NAME} Qt5::Core  Qt5::Qml Qt5::Quick)
target_link_libraries(${LIB_NAME} -lKeepHeartProxy -lutils -lcjson -lpthread )


if( ${PUSH_REMOTE} EQUAL 1)

else()
    add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
        COMMAND  cp ${CMAKE_BINARY_DIR}/test/${LIB_NAME}  ${PACK_TEST_DIR}
        COMMAND echo "${LIB_NAME}" cp to pack  success
    )
endif()





