    .syntax    unified
    .arch    armv6-m
    .text
    .thumb
    .thumb_func
    .align    2    
    .globl LLD_PDU_TX_FLUSH_LIST_PATCH
    .type LLD_PDU_TX_FLUSH_LIST_PATCH,%function
LLD_PDU_TX_FLUSH_LIST_PATCH:
    MOV R1,R4
    BL lld_pdu_tx_flush_patch_c
    MOV R4,R0
    LDR R0,=0x18641
    BX R0
    
    .end
    