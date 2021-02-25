add_sources(USR_FLASH_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/flash_base.c
${CMAKE_CURRENT_SOURCE_DIR}/flash_wrapper.c
)


add_sources(DTM_FLASH_SRCS
${CMAKE_CURRENT_SOURCE_DIR}/flash_base.c
${CMAKE_CURRENT_SOURCE_DIR}/flash_wrapper_dtm.c
)


target_include_directories(usr_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)