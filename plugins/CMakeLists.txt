
function(push_to_target )
    if( ${PUSH_REMOTE} EQUAL 1)

    else()
        add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
            COMMAND  cp ${CMAKE_BINARY_DIR}/plugins/${LIB_DIR}/lib${LIB_NAME}.so  ${PACK_PLUGINS_DIR}
            COMMAND echo "${LIB_NAME}" cp to pack  success
        )
    endif()

endfunction()


# if(${BUILD_FOR} STREQUAL "LGNJ01")
#     add_subdirectory(bd37033)
#     # add_subdirectory(MusicClient)
#     add_subdirectory(VoiceModule)
#     add_subdirectory(McuParser_LGNJ)
    
# endif()
# #GLB 项目的插件
# if(${BUILD_FOR} STREQUAL "GLBM01")
#     add_subdirectory(PluginConfig)
#     add_subdirectory(SysCtrl)
#     add_subdirectory(McuParser)
#     add_subdirectory(Icm680)
#     # add_subdirectory(FdbusDemo)
#     add_subdirectory(glbErrCodePrase) 
# endif()

#共用的插
# add_subdirectory(Pt2314)
# add_subdirectory(MplayerClient)  这个还不用

add_subdirectory(PluginConfig)