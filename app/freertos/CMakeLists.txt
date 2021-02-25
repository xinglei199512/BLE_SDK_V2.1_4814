set(MAIN_SRC 
${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

set_property(GLOBAL PROPERTY RTOS_APP_SRCS)
add_library(rtos_app_inc INTERFACE)
macro(rtos_app_target_build tgt)
get_target_property(rtos_app_inc_list rtos_app_inc INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND ${tgt}_inc_list ${rtos_app_inc_list})

get_property(RTOS_APP_SRCS_LIST GLOBAL PROPERTY RTOS_APP_SRCS)
app_target_build(${tgt} ${ARGN} ${RTOS_APP_SRCS_LIST} ${MAIN_SRC})
target_link_libraries(${tgt} rtos_app_inc)


endmacro(rtos_app_target_build)

target_include_directories(rtos_app_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(src)
add_subdirectory(utils)
add_subdirectory(ble)
add_subdirectory(mesh)
