set(SRC
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tmall)
