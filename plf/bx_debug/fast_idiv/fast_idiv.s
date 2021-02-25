
;;;;;;;;;;;;;;;;;;;NOTE:;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;When use this feature , please change the ROM TABLE
;delete __aeabi_uidiv,__aeabi_uidivmod,__aeabi_idiv,__aeabi_idivmod


    PRESERVE8
	THUMB
	AREA    |.text|, CODE, READONLY


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;uidiv optimize for loop
;source code in rom_180529_2400\fw_full.asm\__aeabi_uidiv
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
__aeabi_uidiv PROC
        PUSH     {r4,r5,lr}
        ;judge valid
        CMP      r0,r1
        BCS      DB_OP
        MOV      r1,r0      ;r1 = r0 % r1
        MOVS     r0,#0      ;r0 = 0
        POP      {r4,r5,pc}
        ;normal operation
DB_OP   MOV      r3,r1
        MOV      r1,r0
        MOVS     r0,#0
        MOVS     r2,#0x20   ;r2=32
        MOVS     r4,#1
        ;first  unconditional branch
        MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=31
        ;loop : r2=31
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_31 
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_31   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=30
        ;loop : r2=30
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_30
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_30   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=29
        ;loop : r2=29
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_29
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_29   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=28
        ;loop : r2=28
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_28
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_28   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=27
        ;loop : r2=27
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_27
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_27   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=26
        ;loop : r2=26
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_26
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_26   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=25
        ;loop : r2=25
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_25
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_25   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=24
        ;loop : r2=24
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_24
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_24   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=23
        ;loop : r2=23
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_23
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_23   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=22
        ;loop : r2=22
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_22
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_22   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=21
        ;loop : r2=21
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_21
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_21   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=20
        ;loop : r2=20
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_20
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_20   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=19
        ;loop : r2=19
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_19
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_19   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=18
        ;loop : r2=18
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_18
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_18   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=17
        ;loop : r2=17
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_17
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_17   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=16
        ;loop : r2=16
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_16
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_16   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=15
        ;loop : r2=15
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_15
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_15   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=14
        ;loop : r2=14
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_14
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_14   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=13
        ;loop : r2=13
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_13
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_13   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=12
        ;loop : r2=12
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_12
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_12   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=11
        ;loop : r2=11
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_11
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_11   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=10
        ;loop : r2=10
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_10
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_10   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=09
        ;loop : r2=09
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_09
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_09   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=08
        ;loop : r2=08
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_08
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_08   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=07
        ;loop : r2=07
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_07
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_07   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=06
        ;loop : r2=06
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_06
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_06   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=05
        ;loop : r2=05
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_05
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_05   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=04
        ;loop : r2=04
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_04
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_04   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=03
        ;loop : r2=03
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_03
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_03   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=02
        ;loop : r2=02
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_02
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_02   MOV      r5,r2
        SUBS     r2,r2,#1   ;r2=01
        ;loop : r2=01
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_01
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_01   MOV      r5,r2      ;r5=01
        SUBS     r2,r2,#1   ;r2=00
        ;loop : r2=00
        MOV      r5,r1
        LSRS     r5,r5,r2
        CMP      r5,r3
        BCC      DB_00
        MOV      r5,r3
        LSLS     r5,r5,r2
        SUBS     r1,r1,r5
        MOV      r5,r4
        LSLS     r5,r5,r2
        ADDS     r0,r0,r5
DB_00   MOV      r5,r2      ;r5=00
        SUBS     r2,r2,#1   ;r2=-1
        ;exit
        POP      {r4,r5,pc}
    ENDP
    ALIAS  __aeabi_uidiv , __aeabi_uidivmod
    EXPORT __aeabi_uidiv
    EXPORT __aeabi_uidivmod
    ;EXPORT DEBUG_UIDIV   


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;idiv optimize for loop
;source code in rom_180529_2400\fu_full.asm\__aeabi_idiv
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
__aeabi_idiv PROC
        PUSH     {r4-r6,lr}
        MOVS     r4,#0
        MOV      r5,r4
        CMP      r0,#0
        BGE      DC_14 ; __aeabi_idiv + 14
        MOVS     r4,#1
        RSBS     r0,r0,#0
DC_14   CMP      r1,#0
        BGE      DC_22 ; __aeabi_idiv + 22
        MOVS     r5,#1
        RSBS     r1,r1,#0
DC_22   BL       __aeabi_uidiv ; 0x1a00
        CMP      r4,r5
        BEQ      DC_32 ; __aeabi_idiv + 32
        RSBS     r0,r0,#0
DC_32   CMP      r4,#0
        BEQ      DC_38 ; __aeabi_idiv + 38
        RSBS     r1,r1,#0
DC_38   POP      {r4-r6,pc}
    ENDP
    ALIAS  __aeabi_idiv , __aeabi_idivmod
    EXPORT __aeabi_idiv
    EXPORT __aeabi_idivmod 
    ;EXPORT DEBUG_IDIV

    
    
    END
    
    
    
    
    
