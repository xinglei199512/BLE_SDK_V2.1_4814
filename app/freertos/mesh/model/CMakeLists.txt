set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/mesh_model.c
#${CMAKE_CURRENT_SOURCE_DIR}/config_client.c
#${CMAKE_CURRENT_SOURCE_DIR}/config_server.c
#${CMAKE_CURRENT_SOURCE_DIR}/sig_msg.c
${CMAKE_CURRENT_SOURCE_DIR}/model_common.c
${CMAKE_CURRENT_SOURCE_DIR}/model_status_switch.c
${CMAKE_CURRENT_SOURCE_DIR}/model_msg.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(api)
add_subdirectory(generics)
add_subdirectory(foundation_models)
add_subdirectory(custom_models)
add_subdirectory(light_mode)
add_subdirectory(time_and_scenes)
#add_subdirectory(sensors)
#add_subdirectory(heartbeat)
#add_subdirectory(health)
