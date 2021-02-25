/*
 * pgspy.c
 *
 *  Created on: 2018Äê7ÔÂ12ÈÕ
 *      Author: jiachuang
 */

#include "reg_sysc_cpu.h"
#include "stdint.h"
#include "plf.h"

void pgspy_monitor(void* start_addr , void* end_addr)
{
    sysc_cpu_pgspy_addr0_l_set((uint32_t)start_addr);
    sysc_cpu_pgspy_addr0_h_set((uint32_t)end_addr);
    sysc_cpu_pgspy_en_set(1<<0);
    sysc_cpu_pgspy_intr_mask_set(1<<0);
    NVIC_EnableIRQ(PGSPY_IRQn);
}


void PGSPY_IRQHandler()
{
    while(*(volatile uint32_t *)0 != 0)
    {
        ;
    }
}




uint32_t test_inc_cnt = 0;
uint32_t test_isr = 0;

void pgspy_test()
{
    uint8_t *start = (uint8_t *)&test_isr;
    uint8_t *end   = (uint8_t *)&test_isr;
    end += 4;
    pgspy_monitor(start , end);
    while(*(volatile uint32_t *)0 != 0)
    {
        test_inc_cnt ++;
        BX_DELAY_US(1000);
        if(test_inc_cnt >= 2000)
        {
            test_isr = 0xA0A0A0A0;
        }
    }
}


