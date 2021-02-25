add_sources(PLF_00_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/rc_calib.c
${CMAKE_CURRENT_SOURCE_DIR}/true_random.c
)

target_include_directories(inc_00
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)  