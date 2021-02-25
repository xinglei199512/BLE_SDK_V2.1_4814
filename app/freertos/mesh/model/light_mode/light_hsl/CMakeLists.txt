set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/server/light_hsl_server.c
${CMAKE_CURRENT_SOURCE_DIR}/client/light_hsl_client.c
${CMAKE_CURRENT_SOURCE_DIR}/server/light_hsl_hue_server.c
${CMAKE_CURRENT_SOURCE_DIR}/server/light_hsl_saturation_server.c
${CMAKE_CURRENT_SOURCE_DIR}/server/light_hsl_setup_server.c
${CMAKE_CURRENT_SOURCE_DIR}/light_hsl_msg_handler.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(client)
add_subdirectory(server)

