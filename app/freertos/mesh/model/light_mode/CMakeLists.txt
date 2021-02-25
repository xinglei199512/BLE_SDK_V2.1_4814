set(SRC
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(light_ctl)
add_subdirectory(light_hsl)
add_subdirectory(light_lightness)
