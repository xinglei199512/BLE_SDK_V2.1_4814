set(SRC
#${CMAKE_CURRENT_SOURCE_DIR}/XXXXXX.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(api)
add_subdirectory(keys)
add_subdirectory(node_save)