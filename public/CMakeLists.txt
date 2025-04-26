
function(push_to_target )

    add_custom_command(TARGET "${LIB_NAME}" POST_BUILD
        COMMAND  cp ${CMAKE_BINARY_DIR}/public/${LIB_DIR}/lib${LIB_NAME}.so  ${PACK_PUBLIC_DIR}
        COMMAND echo "${LIB_NAME}" cp to pack  success
    )

endfunction()

add_subdirectory(utils)
add_subdirectory(cjson)
add_subdirectory(parse)

