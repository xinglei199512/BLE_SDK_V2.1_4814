set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/server/tmall_model_server.c
${CMAKE_CURRENT_SOURCE_DIR}/client/tmall_model_client.c
${CMAKE_CURRENT_SOURCE_DIR}/tmall_model_msg_handler.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(client)
add_subdirectory(server)

