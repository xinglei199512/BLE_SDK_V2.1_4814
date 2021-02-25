set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/mesh_node_base.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_node_config.c
${CMAKE_CURRENT_SOURCE_DIR}/beacon_config.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_kr_client.c
#${CMAKE_CURRENT_SOURCE_DIR}/stack_mem_cfg.c 
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)


