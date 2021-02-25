    .syntax    unified
    .arch    armv6-m
    .section n_xip_section
    .thumb
    .thumb_func
    .align    2   
    .globl SLAVE_FINETIMECNT_PATCH
    .type SLAVE_FINETIMECNT_PATCH,%function
SLAVE_FINETIMECNT_PATCH:
    MRS r1,MSP
    BL finetimecnt_recalculate
    LDR r2,=0x17677
    BX r2
    .end
    