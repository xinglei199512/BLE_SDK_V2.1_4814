set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/config_server.c
${CMAKE_CURRENT_SOURCE_DIR}/config_server_feature.c
${CMAKE_CURRENT_SOURCE_DIR}/config_server_heartbeat.c
${CMAKE_CURRENT_SOURCE_DIR}/config_server_key.c
${CMAKE_CURRENT_SOURCE_DIR}/config_server_misc.c
${CMAKE_CURRENT_SOURCE_DIR}/config_server_pub_sub.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

