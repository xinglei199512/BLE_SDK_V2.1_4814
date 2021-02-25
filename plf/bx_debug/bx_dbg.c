/*
 * bx_dbg.c
 *
 *  Created on: 2016Äê3ÔÂ22ÈÕ
 *      Author: Administrator
 */
#define __RAM_CODE__
#include "log.h"
#include "arch.h"
#include "apollo_00.h"
#include "bx_dbg.h"
#include <stdarg.h>
#include "compiler_flag.h"
#include "bx_log.h"

int SEGGER_RTT_vprintf(unsigned BufferIndex, const char * sFormat, va_list * pParamList);
USED_FLAG XIP_SECTION void flash_func(){}

N_XIP_SECTION void __aeabi_assert(const char *expr,const char *file,int line)
{
    GLOBAL_INT_STOP();
    LOG(LOG_LVL_ERROR,"Assertion Failed: file %s, line %d, %s\n",file,line,expr);
    //while(*(__IO uint32_t *)0 != 0);
    while(1);
    //__BKPT(0);
}

N_XIP_SECTION void rwip_assert_c(uint32_t lvl,uint32_t param0,uint32_t param1,uint32_t lr)
{
    LOG(lvl,"lvl:%x,lr=0x%08x,param0=0x%x,param1=0x%x\n",lvl,lr,param0,param1);
    if(lvl==LVL_ERROR)
    {
        GLOBAL_INT_STOP();
        while(1);
    }
}

N_XIP_SECTION void rtt_output(int8_t level,bool linefeed,const char *format,...)
{
    va_list args;
    va_start(args,format);
    SEGGER_RTT_vprintf(0,format,&args);
    if(linefeed)
    {
        SEGGER_RTT_vprintf(0,"\n",NULL);
    }
    va_end(args);
}

void bx_simu_finish()
{
    *(uint32_t *)SIM_REPORT_BASE = 0x30;
}
void bx_simu_fail()
{
    *(uint32_t *)SIM_REPORT_BASE = 0x20;
}
void bx_simu_pass()
{
    *(uint32_t *)SIM_REPORT_BASE = 0x10;
}
struct
{
    uint32_t msp;
    uint32_t psp;
    uint32_t lr;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
}hardfault_env;
N_XIP_SECTION void hardfault_print(void)
{
    enum{
    R0_INSTACK,
    R1_INSTACK,
    R2_INSTACK,
    R3_INSTACK,
    R12_INSTACK,
    LR_INSTACK,
    PC_INSTACK,
    xPSR_INSTACK,
    };

    uint32_t msp = hardfault_env.msp;
    uint32_t psp = hardfault_env.psp;
    uint32_t lr = hardfault_env.lr;
    uint32_t r4 = hardfault_env.r4;
    uint32_t r5 = hardfault_env.r5;
    uint32_t r6 = hardfault_env.r6;
    uint32_t r7 = hardfault_env.r7;
    LOG(LOG_LVL_ERROR, "!!!!!!!!!!HardFault Handler is triggered!!!!!!!!!!\r\n");
    LOG(LOG_LVL_ERROR, "Prolog:\r\n");
    LOG(LOG_LVL_ERROR, "R4   = 0x%08x\r\n", r4);
    LOG(LOG_LVL_ERROR, "R5   = 0x%08x\r\n", r5);
    LOG(LOG_LVL_ERROR, "R6   = 0x%08x\r\n", r6);
    LOG(LOG_LVL_ERROR, "R7   = 0x%08x\r\n", r7);
    LOG(LOG_LVL_ERROR, "lr   = 0x%08x\r\n", lr);
    LOG(LOG_LVL_ERROR, "msp  = 0x%08x\r\n", msp);
    LOG(LOG_LVL_ERROR, "psp  = 0x%08x\r\n", psp);
    uint32_t *sp = 0;
    if(lr==0xfffffffd)
    {
        sp = (uint32_t*)psp;
        LOG(LOG_LVL_ERROR,"PSP Stack Info:\r\n");
    }
    else{
        sp = (uint32_t*)msp;
        LOG(LOG_LVL_ERROR,"MSP Stack Info:\r\n");
    }

    // Try to dump
    LOG(LOG_LVL_ERROR, "R0   = 0x%08x\r\n", sp[R0_INSTACK]);
    LOG(LOG_LVL_ERROR, "R1   = 0x%08x\r\n", sp[R1_INSTACK]);
    LOG(LOG_LVL_ERROR, "R2   = 0x%08x\r\n", sp[R2_INSTACK]);
    LOG(LOG_LVL_ERROR, "R3   = 0x%08x\r\n", sp[R3_INSTACK]);
    LOG(LOG_LVL_ERROR, "R12  = 0x%08x\r\n", sp[R12_INSTACK]);
    LOG(LOG_LVL_ERROR, "LR   = 0x%08x\r\n", sp[LR_INSTACK]);
    LOG(LOG_LVL_ERROR, "PC   = 0x%08x\r\n", sp[PC_INSTACK]);
    LOG(LOG_LVL_ERROR, "xPSR = 0x%08x\r\n", sp[xPSR_INSTACK]);
}
void hardfault_env_save(void*,uint32_t,uint32_t);

N_XIP_SECTION void HardFault_Handler(void)
{
    uint32_t canonical_frame_addr = __get_MSP() + 8;
    uint32_t *lr_addr = (uint32_t *)(canonical_frame_addr - 4);
    hardfault_env_save(&hardfault_env,canonical_frame_addr,*lr_addr);
    hardfault_print();
    while(*(volatile uint8_t *)1);
}



