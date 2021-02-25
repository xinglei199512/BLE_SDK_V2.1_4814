set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/server/light_lightness_server.c
${CMAKE_CURRENT_SOURCE_DIR}/client/light_lightness_client.c
${CMAKE_CURRENT_SOURCE_DIR}/server/light_lightness_setup_server.c
${CMAKE_CURRENT_SOURCE_DIR}/light_lightness_msg_handler.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)


add_subdirectory(client)
add_subdirectory(server)

