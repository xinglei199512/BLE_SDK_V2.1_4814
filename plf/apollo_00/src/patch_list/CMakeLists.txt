set(SRC

#${CMAKE_CURRENT_SOURCE_DIR}/rf_calib_sw_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/tx_test_end.c
${CMAKE_CURRENT_SOURCE_DIR}/adv_int.c
${CMAKE_CURRENT_SOURCE_DIR}/h4tl_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/set_adv_payload_31Byte_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/prog_latency_patch.c

${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/h4tl_patch_asm.s
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/h4tl_read_start_patch_asm.s
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/task_id_patch_asm.s
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/lld_pdu_tx_flush_list_patch_asm.s
${CMAKE_CURRENT_SOURCE_DIR}/lld_pdu_tx_flush_list_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/llm_end_evt_defer_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/h4tl_read_start_skip_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/task_id_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/rc32k_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/derivation_calib_switch.c
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/rc32k_patch_asm.s
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/lld_slave_instant_patch_asm.s
${CMAKE_CURRENT_SOURCE_DIR}/lld_slave_instant_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/slave_finetimecnt_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/slave_finetimecnt_patch_asm.s
${CMAKE_CURRENT_SOURCE_DIR}/smpc_construct_id_addr_type_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/smpc_construct_id_addr_type_patch_asm.s
${CMAKE_CURRENT_SOURCE_DIR}/scan_timeout_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/adv_report_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/adv_report_patch_asm.s
${CMAKE_CURRENT_SOURCE_DIR}/scan_adv_rx_threshold_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/lld_scan_stop.c
${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_C_COMPILER_ID}/lld_scan_stop.s
${CMAKE_CURRENT_SOURCE_DIR}/lld_evt_schedule_patch.c
${CMAKE_CURRENT_SOURCE_DIR}/slave_latency_patch.c
)

add_sources(PLF_00_SRCS ${SRC})

target_include_directories(inc_00
INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
