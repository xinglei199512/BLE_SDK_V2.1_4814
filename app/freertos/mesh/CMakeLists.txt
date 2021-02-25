
add_library(mesh_inc INTERFACE)
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(GLOBAL PROPERTY MESH_SRCS)

macro(mesh_target_build tgt)
get_target_property(mesh_inc_list mesh_inc INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND ${tgt}_inc_list ${mesh_inc_list})

get_property(MESH_SRCS_LIST GLOBAL PROPERTY MESH_SRCS)
rtos_app_target_build(${tgt} ${MESH_SRCS_LIST} ${ARGN})
target_link_libraries(${tgt} mesh_inc)


endmacro(mesh_target_build)

add_subdirectory(adapter)
add_subdirectory(mesh_test)
add_subdirectory(provision)
add_subdirectory(model)
add_subdirectory(gatt)
add_subdirectory(core)
add_subdirectory(uart_ctrl)
add_subdirectory(device_manager)
add_subdirectory(unit_test)
add_subdirectory(examples)


