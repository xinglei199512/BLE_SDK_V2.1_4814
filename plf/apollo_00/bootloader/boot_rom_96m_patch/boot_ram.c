/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "boot_typedef.h"
#include "boot_memory_map.h"
#include "apollo_00.h"
#include "uart_6m_patch.h"


typedef void (*reset_handler_t)(uint32_t,uint32_t,uint32_t,uint32_t);
/*
 * MACROS
 ****************************************************************************************
 */
#define BOOT_HEADER_ATTRIBUTE __attribute__((section("boot_header_area")))
#define BOOT_RAM_IMAGE_SIZE 0x400
#define TO_BE_FILLED        0xffffffff
#define CKECK_PASS          1
#define CKECK_FAIL          0xFF
#define DBG1 (*(volatile uint32_t *)0x00130100)

//add to rom use
#define ROM_BASE  0
#define BOOTINIT_FLAG       0x3399

//extern void bx_delay_asm(unsigned int loop);
/**
 * 1. This macro should not be used until awo registers related to AHB_CLK have been set.
 * 2. Never set "a" as a value which is not an integer.
 * 3. Note that "a" should not result in overflow.
 * **/
//#define BX_DELAY_US(a) bx_delay_asm((a)*8)

/*
 * DECLARE
 ****************************************************************************************
 */
int main(void);
void Reset_Handler(void);

/*
 * VARIABLE DEFINITIONS
 ****************************************************************************************
 */

static uint32_t *vec_int_base;  //cannot use local variable! will clear MSP!!
/*
 * CONST
 ****************************************************************************************
 */
const boot_header_t boot_header BOOT_HEADER_ATTRIBUTE = {
        .bx_flag = BX_FLAG_CONST,
        .base_addr = (uint8_t *)UART_DOWNLOAD_EXEC_BASE,
        .length = TO_BE_FILLED,
        .entry_point = Reset_Handler,    //exception_exit   ic A version chip
};


void SystemInit(uint32_t param0,uint32_t param1,uint32_t param2,uint32_t param3)
{
//    __DMB();
//    SCB->VTOR = UART_DOWNLOAD_EXEC_BASE;
//    __DSB();
}
//<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
//MUSTNOT IN INTERRUPT
void start_rom_run_function(void)
{
    //RUN APPLICATION
    vec_int_base = (uint32_t *)ROM_BASE;
    __set_MSP(*vec_int_base);
    reset_handler_t *reset_handler = (reset_handler_t *)(vec_int_base + 1);
    // Jump to user's Reset_Handler
    (*reset_handler)(0,0,0,BOOTINIT_FLAG);        //0x3399 indicate bootram has runned.
}

int main()
{
    DBG1 = 0x22;

    //patch
    *(volatile uint32_t *)0x20133000 = 0x31c;
    *(volatile uint32_t *)0x20133004 = 0x320;
    
    *(volatile uint32_t *)0x20133040 = 0xf1276030;
    *(volatile uint32_t *)0x20133044 = 0xf000feb3;
    
    *(volatile uint32_t *)0x20133080 = 0x03;
    //patch
    
    
    start_rom_run_function();

    return 0;
}



