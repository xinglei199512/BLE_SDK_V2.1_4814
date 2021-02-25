set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/client/generic_onoff_client.c
${CMAKE_CURRENT_SOURCE_DIR}/server/generic_onoff_server.c
${CMAKE_CURRENT_SOURCE_DIR}/generic_onoff_msg_handler.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
add_subdirectory(client)
add_subdirectory(server)
