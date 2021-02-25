set(SRC
#${CMAKE_CURRENT_SOURCE_DIR}/mesh_app_action.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_iv_update_test.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_kr_test.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_ttl_test.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
