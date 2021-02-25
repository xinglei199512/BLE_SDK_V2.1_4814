set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/node_save.c
${CMAKE_CURRENT_SOURCE_DIR}/node_save_common.c
${CMAKE_CURRENT_SOURCE_DIR}/node_save_keys.c
${CMAKE_CURRENT_SOURCE_DIR}/node_save_misc.c
${CMAKE_CURRENT_SOURCE_DIR}/node_save_model.c
${CMAKE_CURRENT_SOURCE_DIR}/node_save_scene.c
${CMAKE_CURRENT_SOURCE_DIR}/node_save_scheduler.c
${CMAKE_CURRENT_SOURCE_DIR}/node_save_onpowerup.c
${CMAKE_CURRENT_SOURCE_DIR}/node_save_onoffcount.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)


