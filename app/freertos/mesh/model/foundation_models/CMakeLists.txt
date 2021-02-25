set(SRC
#${CMAKE_CURRENT_SOURCE_DIR}/config_relay.c
#${CMAKE_CURRENT_SOURCE_DIR}/config_composition_data.c
#${CMAKE_CURRENT_SOURCE_DIR}/config_server_virt.c
#${CMAKE_CURRENT_SOURCE_DIR}/config_server_publication.c
#${CMAKE_CURRENT_SOURCE_DIR}/config_server_subscription.c
${CMAKE_CURRENT_SOURCE_DIR}/model_publish.c
#${CMAKE_CURRENT_SOURCE_DIR}/config_proxy.c
${CMAKE_CURRENT_SOURCE_DIR}/foundation_common.c
${CMAKE_CURRENT_SOURCE_DIR}/foundation_msg_handler.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(config)
add_subdirectory(health)

