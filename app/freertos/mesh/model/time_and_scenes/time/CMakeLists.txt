set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/server/time_server.c
${CMAKE_CURRENT_SOURCE_DIR}/client/time_client.c
${CMAKE_CURRENT_SOURCE_DIR}/time_msg_handler.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(client)
add_subdirectory(server)

