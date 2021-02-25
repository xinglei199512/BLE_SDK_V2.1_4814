    .syntax    unified
    .arch    armv6-m
    .section n_xip_section
    .thumb
    .thumb_func
    .align    2   
    .globl  SMPC_CONSTRUCT_ID_ADDR_TYPE_PATCH
    .type SMPC_CONSTRUCT_ID_ADDR_TYPE_PATCH,%function
SMPC_CONSTRUCT_ID_ADDR_TYPE_PATCH:
    PUSH {LR}
    MOV R4,R0
    BL smpc_construct_id_addr_type_patch_c
    POP {PC}
    
    .end
    