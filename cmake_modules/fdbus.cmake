


set(FDBUS_PREFIX /home/ieai/workspace/fdbus_services)

set(FDBUS_LIBPATH ${FDBUS_PREFIX}/fdbus/platform/arm-linux/lib
			${FDBUS_PREFIX}/fdbus/platform/arm-linux/usr/lib
			${FDBUS_PREFIX}/out/lib
			)
			
set(FDBUS_HEADPATH ${FDBUS_PREFIX}/fdbus/platform/arm-linux/include
			${FDBUS_PREFIX}/fdbus/platform/arm-linux/usr/include)

set(FDBUS_INTERFACE ${FDBUS_PREFIX}/services/Interface)


include_directories (
${FDBUS_HEADPATH}
${FDBUS_INTERFACE}
)

 link_directories(${FDBUS_LIBPATH} )


add_definitions(-DCONFIG_PROTOBUF_4_0_X)

# fdbus 是否输出打印
add_definitions("-DCONFIG_LOG_TO_STDOUT")
add_definitions("-DCONFIG_DEBUG_LOG")