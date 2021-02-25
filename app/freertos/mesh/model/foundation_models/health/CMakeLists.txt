set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/server/health_server.c
${CMAKE_CURRENT_SOURCE_DIR}/client/health_client.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(server)
add_subdirectory(client)
