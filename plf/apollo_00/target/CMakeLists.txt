get_property(ROM_00_SRCS_LIST GLOBAL PROPERTY ROM_00_SRCS)
get_property(PLF_00_SRCS_LIST GLOBAL PROPERTY PLF_00_SRCS)
get_property(USR_SRCS_LIST GLOBAL PROPERTY USR_SRCS)
get_property(USR_FLASH_SRCS_LIST GLOBAL PROPERTY USR_FLASH_SRCS)
include_directories(./)
add_executable(apollo_00_full main.c ${ROM_00_SRCS_LIST} ${PLF_00_SRCS_LIST} ${USR_SRCS_LIST} ${USR_FLASH_SRCS_LIST} ${ARGN})
target_compile_definitions(apollo_00_full
PUBLIC HW_BX_VERSION=00 ALL_IN_RAM
)
set(FW_RAM_00_SCATTER_FILE_PATH ${CMAKE_SOURCE_DIR}/plf/apollo_00/config/linker_script/${CMAKE_C_COMPILER_ID}/fw_ram.txt)
target_link_libraries(apollo_00_full fw_inc usr_inc inc_00 ${link_gc_option} ${linkscript_option}${FW_RAM_00_SCATTER_FILE_PATH})
set_target_properties(apollo_00_full PROPERTIES
LINK_DEPENDS ${FW_RAM_00_SCATTER_FILE_PATH}
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/apollo_00_full
)
add_custom_command(TARGET apollo_00_full POST_BUILD
    COMMAND fromelf -c -a -d -e -v -o apollo_00_full.asm apollo_00_full.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/output/apollo_00_full
    COMMENT "Disassembling apollo_00_full.elf to apollo_00_full.asm"
)