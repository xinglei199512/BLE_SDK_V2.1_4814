set(SRC
    #${CMAKE_CURRENT_SOURCE_DIR}/config_client.c
    #${CMAKE_CURRENT_SOURCE_DIR}/config_client_feature.c
    #${CMAKE_CURRENT_SOURCE_DIR}/config_client_heartbeat.c
    #${CMAKE_CURRENT_SOURCE_DIR}/config_client_key.c
    #${CMAKE_CURRENT_SOURCE_DIR}/config_client_misc.c
    #${CMAKE_CURRENT_SOURCE_DIR}/config_client_pub_sub.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

