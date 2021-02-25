/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "boot_typedef.h"
#include "boot_memory_map.h"
#include "flash.h"
#include "flash_cache.h"
#include "apollo_00.h"
//#include "awo.h"
#include "reg_sysc_cpu.h"
#include "reg_sysc_per.h"
#include "uart_download.h"
//#include "clk_gate.h"
#include "dma_for_qspi.h"
#include "qspi.h"
#include "bx_crc32.h"
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


extern void bx_delay_asm(unsigned int loop);
/**
 * 1. This macro should not be used until awo registers related to AHB_CLK have been set.
 * 2. Never set "a" as a value which is not an integer.
 * 3. Note that "a" should not result in overflow.
 * **/
#define BX_DELAY_US(a) bx_delay_asm((a)*8)

/*
 * DECLARE
 ****************************************************************************************
 */
int main(void);
void Reset_Handler(void);
void exception_exit(void);

/*
 * VARIABLE DEFINITIONS
 ****************************************************************************************
 */
uint8_t wait_download_finish = 0;
uint8_t check_result = 0;
uint8_t uart_baud_changed = 0;
volatile uint8_t start_ram_run = 0;
volatile uint8_t fix_warning = 0;

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


/*
static void open_clkgate()
{
    clk_gate_cpu_g1(CPU_CLKG_SET_DMAC);     //open dmac clock
    clk_gate_cpu_g1(CPU_CLKG_SET_QSPI);     //open qspi clock

    clk_gate_per_g0(PER_CLKG0_SET_UART0);   //open uart
    clk_gate_per_g0(PER_CLKG1_SET_UART0);
}
*/

static void open_pinshare()
{
    sysc_cpu_qspi_en_setf(0x3);                         // qspi pin: standard spi enable

    sysc_cpu_func_io10_sel_setf(0);                     //IO_UART0_TXD  funcio 10 = gpio_mux[12]
    sysc_cpu_func_io11_sel_setf(1);                     //IO_UART0_RXD  funcio 11 = gpio_mux[13]

    sysc_cpu_func_io_en_r_setf((1<<10) | (1<<11));
}

void SystemInit(uint32_t param0,uint32_t param1,uint32_t param2,uint32_t param3)
{
    __DMB();
    SCB->VTOR = UART_DOWNLOAD_EXEC_BASE;
    __DSB();
}
//--------------------------
//#define CO_BIT(pos) 					(1UL<<(pos))
//#define CLK_MASK_CLR    			0xaaaaaaaa
//#define CLK_MASK_SET    			(~CLK_MASK_CLR)
//#define PER_CLKG_SET_GPIO           CO_BIT(12)

////#define    REG_GPIO_BASE                  0x20148000          

//#define    REG_GPIO_SWPORTA_DR                 (*(volatile uint32_t *)0x20148000) 
//#define    REG_GPIO_SWPORTA_DDR                (*(volatile uint32_t *)0x20148004) 


//static void clk_gate_per_g1(uint32_t map)
//{
//    sysc_per_clkg1_set(map);
//    if(map & CLK_MASK_CLR)            //clr
//    {
//        while((sysc_per_clkg1_get() & map));
//    }
//    else if(map & CLK_MASK_SET)       //set
//    {
//        while((~ sysc_per_clkg1_get()) & map);
//    }
//    else
//    {
//        //BX_ASSERT(0);
//    }
//}

//static void gpio_clk_gate(void)
//{
//	clk_gate_per_g1(PER_CLKG_SET_GPIO);
//}

//static void io_test(void)
//{

//	gpio_clk_gate();
//		
//	REG_GPIO_SWPORTA_DDR |= 1<<22;
//	//REG_GPIO_SWPORTA_DR  |= 1<<22; //high
//	REG_GPIO_SWPORTA_DR  &= ~(1<<22); //low
//	
//}

//---------------------------


int main()
{
    DBG1 = 0x22;
//    awo_bootram_init();
//    open_clkgate();
    open_pinshare();

//    qspi_rxsample_dly_set(2);       //qspi rx sample delay.

    dmac_init();
    qspi_init(2,1);
    //flash_init();
    //DBG1 = flash_quad_enable();
    
    //crc32 table generate
    crc32_init(CRC32_POLY_VAL);

    uart_download_init();

    //DBG1 = 0x33;
    uart_sync_to_pc();

	//io_test();
	
    while( !uart_baud_changed );
    //DBG1 = 0x44;
    
    // add check flow here   
    uart_download_start();
    
    //wait user action
    while(fix_warning == 0)
    {
        if(wait_download_finish != 0)
        {
            
        }
        if(check_result != 0)
        {
            
        }
        if(start_ram_run != 0)
        {
            BX_DELAY_US(1000);
            start_ram_run_function();
        }
    }
    return 0;
}



