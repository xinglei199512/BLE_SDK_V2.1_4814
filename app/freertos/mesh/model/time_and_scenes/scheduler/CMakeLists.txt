set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/server/scheduler_server.c
${CMAKE_CURRENT_SOURCE_DIR}/client/scheduler_client.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(client)
add_subdirectory(server)
