set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/generic_common.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(generic_onoff)
add_subdirectory(generic_level)
add_subdirectory(generic_transition)
add_subdirectory(generic_power_onoff)
