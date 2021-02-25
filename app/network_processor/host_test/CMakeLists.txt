app_target_build(host_test main.c)
set(FW_RAM_00_SCATTER_FILE_PATH ${PROJECT_SOURCE_DIR}/plf/apollo_00/config/linker_script/${CMAKE_C_COMPILER_ID}/fw_ram.txt)
target_build_dtm(host_test_ram ${FW_RAM_00_SCATTER_FILE_PATH} main.c)

add_custom_command(TARGET host_test_ram POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory host_test_ram
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/output/
)

if(CMAKE_C_COMPILER_ID STREQUAL ARMCC)
add_custom_command(TARGET host_test_ram POST_BUILD
    COMMAND fromelf -c -a -d -e -v -o host_test_ram.asm ../host_test_ram.elf
    COMMAND fromelf --bincombined --output=DTM_RAMCODE.bin ../host_test_ram.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/output/host_test_ram
    COMMENT "Disassembling elf to asm && Converting elf to bin"
)
else()
add_custom_command(TARGET host_test_ram POST_BUILD
    COMMAND arm-none-eabi-objdump -d -x ../host_test_ram.elf > host_test_ram.asm
    COMMAND arm-none-eabi-objcopy -O binary ../host_test_ram.elf DTM_RAMCODE.bin
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/output/host_test_ram
    COMMENT "Disassembling elf to asm && Converting elf to bin"
)
endif()
