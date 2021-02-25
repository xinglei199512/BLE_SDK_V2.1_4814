set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/static_buffer.c
)

add_sources(USR_SRCS ${SRC})
target_include_directories(usr_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
