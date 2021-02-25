set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/mesh_gatt_cfg.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_gatt.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_gatt_filter.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_gatt_proxy.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_gatt_serv_adv.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(api)
add_subdirectory(proxy)
add_subdirectory(provisioning)

