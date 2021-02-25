    .syntax	unified
	.arch	armv6-m
    .section n_xip_section
    .thumb
	.thumb_func
    .align	1    
    .globl SLAVE_EVT_COUNT_JUST_BEFORE_INSTANT
    .type SLAVE_EVT_COUNT_JUST_BEFORE_INSTANT,%function

SLAVE_EVT_COUNT_JUST_BEFORE_INSTANT:
    MOV r1,r6
    BL slave_event_count_just_before_instant_c
    BX r0
    
	.end
    