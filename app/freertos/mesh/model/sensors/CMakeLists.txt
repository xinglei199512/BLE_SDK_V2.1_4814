set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/sensor_server.c
${CMAKE_CURRENT_SOURCE_DIR}/sensor_client.c
${CMAKE_CURRENT_SOURCE_DIR}/sensor_server_setup.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)


