    .syntax	unified
	.arch	armv6-m
    .text
    .thumb
	.thumb_func
    .align	1    
    .globl LLD_SCAN_STOP_PATCH
    .type LLD_SCAN_STOP_PATCH,%function
LLD_SCAN_STOP_PATCH:

    BL lld_scan_stop_patch
    POP      {r4,pc}
    
    .end
    
    
    
    
    
    
