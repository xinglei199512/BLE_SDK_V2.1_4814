set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/provision_api.c
)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
