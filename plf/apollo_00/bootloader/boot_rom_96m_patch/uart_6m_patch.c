/**
 ****************************************************************************************
 *
 * @file   uart_6m_patch.c
 *
 * @brief  .
 *
 * @author  Hui Chen
 * @date    2019-03-11 16:13
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */
#ifndef PLF_APOLLO_00_SRC_PATCH_LIST_UART_6M_PATCH_C_
#define PLF_APOLLO_00_SRC_PATCH_LIST_UART_6M_PATCH_C_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "uart_6m_patch.h"
#include "apollo_00.h"

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
extern void bx_delay_asm(unsigned int loop);
/**
 * 1. This macro should not be used until awo registers related to AHB_CLK have been set.
 * 2. Never set "a" as a value which is not an integer.
 * 3. Note that "a" should not result in overflow.
 * **/
#define BX_DELAY_US(a) bx_delay_asm((a)*8) 
 
#define UART_BAUD_6M_PATCH_ADDR     0x31e
/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
__attribute__((section("uart_6m_section")))  __attribute__((used)) void uart_6m_patch_c(void)
{
    *(volatile uint32_t *)0x20201070 = 0x10cf5cb3;
    while(((*(volatile uint32_t *)0x202010b0)&0x20)==0x0);
    BX_DELAY_US(2000);
    *(volatile uint32_t *)0x20201000 = 0x00e00091;
    __asm("nop");
    __asm("nop");
    *(volatile uint32_t *)0x20201000 = 0x00e00092;
    *(volatile uint32_t *)0x20149010 = 0x00000a00;
    __asm("nop");
    __asm("nop");
    *(volatile uint32_t *)0x20149000 = 0x00000200;
    __asm("nop");
    __asm("nop");
    *(volatile uint32_t *)0x20149010 = 0x00000500;    
} 

#endif /* PLF_APOLLO_00_SRC_PATCH_LIST_UART_6M_PATCH_C_ */ 

