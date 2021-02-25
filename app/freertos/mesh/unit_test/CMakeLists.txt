set(SRC
#${CMAKE_CURRENT_SOURCE_DIR}/security_test/aes_128_test.c
${CMAKE_CURRENT_SOURCE_DIR}/security_test/aes_ccm_test.c
#${CMAKE_CURRENT_SOURCE_DIR}/security_test/aes_cmac_test.c

)

add_sources(MESH_SRCS ${SRC})
target_include_directories(mesh_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)



