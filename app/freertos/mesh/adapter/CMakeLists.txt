set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/mesh_queued_msg.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_sched.c
${CMAKE_CURRENT_SOURCE_DIR}/timer_wrapper.c 
${CMAKE_CURRENT_SOURCE_DIR}/os_wrapper.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_ble_time.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)



