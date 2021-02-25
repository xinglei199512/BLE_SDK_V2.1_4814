    .syntax    unified
    .arch    armv6-m
    .text
    .thumb
    .thumb_func
    .align    1    
    .globl LLD_EVT_DRIFT_COMPUTE_PATCH
    .type LLD_EVT_DRIFT_COMPUTE_PATCH,%function
LLD_EVT_DRIFT_COMPUTE_PATCH:
    BL lld_evt_drift_compute_patch
    LDR r2,[sp,#4]
    ADD sp,sp,#8
    BX r2
    
    .end
    