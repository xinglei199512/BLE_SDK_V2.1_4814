    .syntax    unified
    .arch    armv6-m
    .text
    .thumb
    .thumb_func
    .align    2    
    .globl LLM_ADV_REPORT_SEND_ADV_RX_TIME_GET
    .type LLM_ADV_REPORT_SEND_ADV_RX_TIME_GET,%function
LLM_ADV_REPORT_SEND_ADV_RX_TIME_GET:
    LDR R1,[sp,#0x14]
    BL adv_rx_time_get_patch_c
    LDR R0,=0x19453
    BX R0
    
    
    .end
    