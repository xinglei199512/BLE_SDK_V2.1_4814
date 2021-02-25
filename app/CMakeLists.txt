set(FW_USR_00_SCATTER_FILE_PATH ${PROJECT_SOURCE_DIR}/plf/apollo_00/config/linker_script/${CMAKE_C_COMPILER_ID}/fw_usr.txt)

get_target_property(fw_inc_list fw_inc INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(usr_inc_list usr_inc INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(inc_00_list inc_00 INTERFACE_INCLUDE_DIRECTORIES)

macro(app_target_build tgt)
target_build_with_link_script(${tgt} ${FW_USR_00_SCATTER_FILE_PATH} ${ARGN})
get_target_property(${tgt}_orig_src_list ${tgt} SOURCES)
foreach(src_path ${${tgt}_orig_src_list})
    if(NOT IS_ABSOLUTE ${src_path})
        string(CONCAT src_path ${CMAKE_CURRENT_SOURCE_DIR} "/" ${src_path})
    endif(NOT IS_ABSOLUTE ${src_path})
    list(APPEND ${tgt}_src_list ${src_path})
endforeach(src_path)
get_target_property(${tgt}_app_inc_list ${tgt} INCLUDE_DIRECTORIES)

list(APPEND ${tgt}_inc_list ${${tgt}_app_inc_list} ${inc_00_list} ${usr_inc_list} ${fw_inc_list})

file(WRITE ${CMAKE_BINARY_DIR}/output/lists/${tgt}/project_path.txt "${CMAKE_CURRENT_SOURCE_DIR}" )
file(WRITE ${CMAKE_BINARY_DIR}/output/lists/${tgt}/srcs.txt "${${tgt}_src_list}")
file(WRITE ${CMAKE_BINARY_DIR}/output/lists/${tgt}/inc_dirs.txt "${${tgt}_inc_list}")

add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${tgt}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/output/
)

if(CMAKE_C_COMPILER_ID STREQUAL ARMCC)

add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND fromelf -c -a -d -e -v -o ${tgt}.asm ../${tgt}.elf
    COMMAND fromelf --i32combined --output=${tgt}.hex ../${tgt}.elf
    COMMAND fromelf --bincombined --output=${tgt}_ota.bin ../${tgt}.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/output/${tgt}
    COMMENT "Disassembling elf to asm && Converting elf to hex"
)
else()
add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND arm-none-eabi-objdump -d -x ../${tgt}.elf > ${tgt}.asm
    COMMAND arm-none-eabi-objcopy -O ihex ../${tgt}.elf ${tgt}.hex
    COMMAND arm-none-eabi-objcopy -O binary ../${tgt}.elf ${tgt}_ota.bin
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/output/${tgt}
    COMMENT "Disassembling elf to asm && Converting elf to hex"
)
endif()

add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND ./boot_ram_config.exe config.ini boot_ram.bin 
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tools/image_tool_v2/
    COMMENT "Generating boot_ram.hex from config.ini"
)

add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND ${PROJECT_SOURCE_DIR}/tools/image_tool_v2/srec_cat.exe ${tgt}.hex -Intel ${PROJECT_SOURCE_DIR}/tools/image_tool_v2/boot_ram.hex -Intel -o ${tgt}_with_bootloader.hex -Intel
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/output/${tgt}
    COMMENT "Merging bootloader and user image into a single hex"
)
endmacro(app_target_build)

macro(target_build_with_link_script tgt ld)
get_property(PLF_00_SRCS_LIST GLOBAL PROPERTY PLF_00_SRCS)
get_property(USR_SRCS_LIST GLOBAL PROPERTY USR_SRCS)
get_property(USR_FLASH_SRCS_LIST GLOBAL PROPERTY USR_FLASH_SRCS)
include_directories(./)
add_executable(${tgt} EXCLUDE_FROM_ALL ${PLF_00_SRCS_LIST} ${USR_SRCS_LIST} ${USR_FLASH_SRCS_LIST} ${ARGN})
target_compile_definitions(${tgt}
PUBLIC HW_BX_VERSION=00 
)
target_link_libraries(${tgt} ${PROJECT_SOURCE_DIR}/plf/apollo_00/config/rom_syms_${CMAKE_C_COMPILER_ID}.txt fw_inc usr_inc inc_00 ${link_gc_option} ${linkscript_option}${ld})
set_target_properties(${tgt} PROPERTIES
LINK_DEPENDS ${ld}
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/
)
endmacro(target_build_with_link_script)


macro(target_build_dtm tgt ld)
get_property(PLF_00_SRCS_LIST GLOBAL PROPERTY PLF_00_SRCS)
get_property(USR_SRCS_LIST GLOBAL PROPERTY USR_SRCS)
get_property(DTM_FLASH_SRCS_LIST GLOBAL PROPERTY DTM_FLASH_SRCS)
include_directories(./)
add_executable(${tgt} EXCLUDE_FROM_ALL ${PLF_00_SRCS_LIST} ${USR_SRCS_LIST} ${DTM_FLASH_SRCS_LIST} ${ARGN})
target_compile_definitions(${tgt}
PUBLIC HW_BX_VERSION=00 
)
target_link_libraries(${tgt} ${PROJECT_SOURCE_DIR}/plf/apollo_00/config/rom_syms_${CMAKE_C_COMPILER_ID}.txt fw_inc usr_inc inc_00 ${link_gc_option} ${linkscript_option}${ld})
set_target_properties(${tgt} PROPERTIES
LINK_DEPENDS ${ld}
RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output/
)
endmacro(target_build_dtm)


add_subdirectory(freertos)
add_subdirectory(baremetal)
add_subdirectory(network_processor)
