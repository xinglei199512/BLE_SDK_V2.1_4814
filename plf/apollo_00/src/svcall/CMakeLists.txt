set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/svcall.c
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/svcall_asm.s
)

add_sources(PLF_00_SRCS ${SRC})

target_include_directories(inc_00
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)