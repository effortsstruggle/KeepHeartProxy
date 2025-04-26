



set(LIB_NAME "KeepHeartProxy")
set(LIB_DIR "dsrc")

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_PREFIX_PATH "/home/qin/qt5.15.2/")
find_package(QT NAMES  Qt5 REQUIRED COMPONENTS  Core Qml Quick )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS  Core Qml Quick )


# # 头文件
include_directories (
  "./" 
  ${PROJECT_SOURCE_DIR}/public/pluginsManger
  ${PROJECT_SOURCE_DIR}/public/utils
)

file(GLOB_RECURSE SRC  ./*.cpp ./*.c )

# 库目录
link_directories(
   ${CMAKE_BINARY_DIR}/public/utils 
   ${CMAKE_BINARY_DIR}/public/cjson
   )


# 编译生成共享库
add_library("${LIB_NAME}"  SHARED   ${SRC})
target_link_libraries(${LIB_NAME} Qt5::Core  Qt5::Qml Qt5::Quick)
target_link_libraries(${LIB_NAME} -lpthread -lcjson -lutils )


if(PUSH_REMOTE)

else()
  add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
  COMMAND  cp ${CMAKE_BINARY_DIR}/${LIB_DIR}/lib${LIB_NAME}.so  ${PACK_LIBS_DIR}
  COMMAND echo "${LIB_NAME}" cp to pack  success
  )
endif()


