set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/uart_main.c
${CMAKE_CURRENT_SOURCE_DIR}/uart_log_test.c
${CMAKE_CURRENT_SOURCE_DIR}/provisioner_uart_test.c
${CMAKE_CURRENT_SOURCE_DIR}/client_server_uart_test.c
${CMAKE_CURRENT_SOURCE_DIR}/mesh_uart_ctrl.c
#${CMAKE_CURRENT_SOURCE_DIR}/unprov_dev_uart_test.c
${CMAKE_CURRENT_SOURCE_DIR}/uart_debug_test.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

