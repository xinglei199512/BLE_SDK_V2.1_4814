set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/app_keys_dm.c
${CMAKE_CURRENT_SOURCE_DIR}/device_keys_dm.c
${CMAKE_CURRENT_SOURCE_DIR}/network_keys_dm.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

