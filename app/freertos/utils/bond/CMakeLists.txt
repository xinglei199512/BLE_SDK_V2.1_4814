set(SRC
${CMAKE_CURRENT_SOURCE_DIR}/bond_manage.c 
${CMAKE_CURRENT_SOURCE_DIR}/bond_save.c 
${CMAKE_CURRENT_SOURCE_DIR}/bond_save_db_cccd.c
${CMAKE_CURRENT_SOURCE_DIR}/bond_save_flash.c
${CMAKE_CURRENT_SOURCE_DIR}/bond_save_security.c
)

add_sources(RTOS_APP_SRCS ${SRC})
target_include_directories(rtos_app_inc
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)


