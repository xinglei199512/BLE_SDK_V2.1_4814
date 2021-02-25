get_property(ROM_00_SRCS_LIST GLOBAL PROPERTY ROM_00_SRCS)
get_property(PLF_00_SRCS_LIST GLOBAL PROPERTY PLF_00_SRCS)
get_property(USR_SRCS_LIST GLOBAL PROPERTY USR_SRCS)
get_property(USR_FLASH_SRCS_LIST GLOBAL PROPERTY USR_FLASH_SRCS)
include_directories(./ ../)


add_executable(
boot_ram main.c 
adc_battery.c 
bx_crc32.c 
${PLF_00_SRCS_LIST} ${USR_SRCS_LIST} ${USR_FLASH_SRCS_LIST})


target_compile_definitions(boot_ram
PUBLIC HW_BX_VERSION=00 __BOOT_RAM__
)
set(BOOT_RAM_SCATTER_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/boot_ram_link.txt)
target_link_libraries(boot_ram ${PROJECT_SOURCE_DIR}/plf/apollo_00/config/rom_syms_${CMAKE_C_COMPILER_ID}.txt fw_inc usr_inc inc_00 ${link_gc_option} ${linkscript_option}${BOOT_RAM_SCATTER_FILE_PATH})
set_target_properties(boot_ram PROPERTIES
LINK_DEPENDS ${BOOT_RAM_SCATTER_FILE_PATH}
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/boot_ram
)
add_custom_command(TARGET boot_ram POST_BUILD
    COMMAND fromelf -c -a -d -e -v -o boot_ram.asm boot_ram.elf
    COMMAND arm-none-eabi-objcopy -O binary -j boot_ram -j .zero.table boot_ram.elf boot_ram.bin
    COMMAND ${PROJECT_SOURCE_DIR}/tools/image_tool_v2/boot_ram_config ${PROJECT_SOURCE_DIR}/tools/image_tool_v2/config.ini boot_ram.bin
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/output/boot_ram/boot_ram.bin  ${PROJECT_SOURCE_DIR}/tools/image_tool_v2
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/output/boot_ram/boot_ram.asm  ${PROJECT_SOURCE_DIR}/tools/image_tool_v2
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/output/boot_ram/boot_ram.elf  ${PROJECT_SOURCE_DIR}/tools/image_tool_v2
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/output/boot_ram/boot_ram.hex  ${PROJECT_SOURCE_DIR}/tools/image_tool_v2
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/output/boot_ram 
    COMMENT "Disassembling boot_ram.elf to boot_ram.asm && Converting boot_ram.elf to binary file into boot_ram.bin"
)