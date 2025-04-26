
function(push_to_target )
    if( ${PUSH_REMOTE} EQUAL 1)
        add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
        COMMAND  scp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so root@192.168.49.1:/apps/plugins/
        COMMAND  cp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so  ${PACK_PLUGINS_DIR}
        COMMAND echo "${LIB_NAME}" push remote success
    )
else()
    add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
        COMMAND  cp ${CMAKE_BINARY_DIR}/plugins/lib${LIB_NAME}.so  ${PACK_PLUGINS_DIR}
        COMMAND echo "${LIB_NAME}" cp to pack  success
    )
endif()

endfunction()

link_directories("/home/ieai/workspace/updateMake/user-resource/GLBM01/libs/")  # cjson 库在这里
#  有些不需要编译的就注释调，需要编译再打开

# add_subdirectory(Rad5807plugin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
if(${BUILD_FOR} STREQUAL "LGNJ01")
    add_subdirectory(bd37033)
    # add_subdirectory(MusicClient)
    add_subdirectory(VoiceModule)
    add_subdirectory(McuParser_LGNJ)
    
endif()

#GLB 项目的插件
if(${BUILD_FOR} STREQUAL "GLBM01")
    add_subdirectory(PluginConfig)
    add_subdirectory(SysCtrl)
    add_subdirectory(McuParser)
    add_subdirectory(Icm680)
    # add_subdirectory(FdbusDemo)
    add_subdirectory(glbErrCodePrase) 
endif()


#共用的插件


add_subdirectory(FM_Si475x)
add_subdirectory(Pt2314)
# add_subdirectory(MplayerClient)  这个还不用

