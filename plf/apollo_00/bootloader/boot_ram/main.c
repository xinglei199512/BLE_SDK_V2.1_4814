#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "boot_ram.h"
#include "boot_memory_map.h"
#include "apollo_00.h"
#include "sysctrl.h"
#include "clk_gate.h"
#include "pshare.h"
#include "app_qspi_wrapper.h"
#include "app_dmac_wrapper.h"
#include "flash.h"
#include "ll.h"
#include "adc_battery.h"
#include "rst_gen.h"
#include "app_wdt.h"
#include "ota_image.h"
#include "flash.h"
#include "bx_crc32.h"

//#define ENABLE_LOW_BATTERY_WAIT
//#define ENABLE_BOOTRAM_WDT

#define BOOT_HEADER_ATTRIBUTE __attribute__((section("boot_header_area")))
#define TO_BE_FILLED        0xffffffff
void main(void);

boot_ram_header_t boot_ram_head BOOT_HEADER_ATTRIBUTE= {
    .header = {
        .bx_flag = BX_FLAG_CONST,
        .base_addr = (uint8_t *)RAM_CODE_EXEC_BASE,
        .length = TO_BE_FILLED,
        .entry_point = main,
    },
};

ota_image_header_t image1_header;

#define FLASH_SECTOR_SIZE   0x1000
#define OTA_BUFF_LENGTH     0x1000
#define OTA_BUFF_ADDR       0x120000
uint8_t *read_buf = (uint8_t *)OTA_BUFF_ADDR;
uint32_t ota_debug_show = 0;

bool quad_enable_read(quad_status_rw_t *ptr)
{
    uint8_t status[ptr->status_length];
    flash_read_status_reg(ptr->cmd,status,ptr->status_length);
    return status[ptr->quad_bit_offset/8] & 1 << ptr->quad_bit_offset%8;
}

void quad_enable_write(quad_status_rw_t *ptr)
{
    uint8_t status[ptr->status_length];
    memset(status,0,ptr->status_length);
    status[ptr->quad_bit_offset/8] = 1 << ptr->quad_bit_offset%8;
    flash_write_status_reg(ptr->cmd,status,ptr->status_length);
}

static void quad_enable()
{
    if(quad_enable_read(&boot_ram_head.flash.quad_enable_config.read)==false)
    {
        quad_enable_write(&boot_ram_head.flash.quad_enable_config.write);
    }
}


void zero_init()
{
    typedef struct{
        void *base;
        uint32_t length;
    }bss_info_t;
    extern bss_info_t __zero_table_start__;
    extern bss_info_t __zero_table_end__;
    bss_info_t *bss_info = &__zero_table_start__;
    for(bss_info = &__zero_table_start__;bss_info<&__zero_table_end__;++bss_info)
    {
        memset(bss_info->base,0,bss_info->length);
    }
}


void bootram_wdt_open(void)
{
    //set clk
    srst_cpu(WDT_SRST_CPU);
    sysc_cpu_clkg1_set(CPU_CLKG_SET_WDT);
    //set parameter
    reg_wdt_t *reg = (reg_wdt_t *)REG_WDT_BASE;
    FIELD_WR(reg, WDT_CR , WDT_RMOD , wdt_Direct_Reset);
    FIELD_WR(reg, WDT_TORR, WDT_TOP , 0x0f);
    FIELD_WR(reg, WDT_CR, WDT_RPL   , wdt_2_pclk_cycles);
    FIELD_WR(reg, WDT_CR, WDT_VAL, 1);
}


bool ota_image1_verify_success(uint32_t image_content_addr , uint32_t length , uint32_t image_crc32)
{
    uint32_t crc= 0xFFFFFFFF;
    uint8_t *buff = (uint8_t*)image_content_addr;
    crc32_init(0x4C11DB7);
    crc=crc32_calc(crc,buff,length);
    return (crc == image_crc32) ? true : false;
}

void debug_show(uint32_t val)
{
    ota_debug_show = val;
}

//ota copy without image header.
void ota_process(void)
{
    uint32_t image0_faddr      = IMAGE_OFFSET_BASE;
    uint32_t image1_head_faddr = boot_ram_head.app.ota_base;
    uint32_t image1_faddr      = image1_head_faddr + sizeof(ota_image_header_t);

    flash_multi_read(image1_head_faddr , sizeof(ota_image_header_t) , (void*)&image1_header);
    debug_show(0x1001);
    //ota available
    if(image1_header.s.valid_flag == OTA_AVAILABLE_FLAG)
    {
        debug_show(0x1002);
        if(ota_image1_verify_success(image1_faddr + FLASH_MAPPED_ADDR , image1_header.s.image_length , image1_header.s.crc32))
        {
            debug_show(0x1003);
            memset(read_buf, 0, OTA_BUFF_LENGTH);
            //calc param
            uint32_t content_size = image1_header.s.image_length;
            uint16_t quotient  = content_size / FLASH_SECTOR_SIZE;
            uint16_t remainder = content_size % FLASH_SECTOR_SIZE;
            uint32_t full_size = content_size + sizeof(ota_image_header_t);
            uint32_t sectors = (full_size / FLASH_SECTOR_SIZE) + ((full_size % FLASH_SECTOR_SIZE) ? 1 : 0);

            //copy block
            uint32_t i = 0;
            for(i=0 ; i < quotient; i++)
            {
                flash_erase     (image0_faddr + i*FLASH_SECTOR_SIZE , Sector_Erase);
                flash_multi_read(image1_faddr + i*FLASH_SECTOR_SIZE , FLASH_SECTOR_SIZE, (void *)&read_buf[0]);
                flash_program   (image0_faddr + i*FLASH_SECTOR_SIZE , FLASH_SECTOR_SIZE, (void *)&read_buf[0]);
            }
            debug_show(0x1004);

            //copy remain
            flash_erase     (image0_faddr + i*FLASH_SECTOR_SIZE , Sector_Erase);
            flash_multi_read(image1_faddr + i*FLASH_SECTOR_SIZE , remainder, (void *)&read_buf[0]);
            flash_program   (image0_faddr + i*FLASH_SECTOR_SIZE , remainder, (void *)&read_buf[0]);
            debug_show(0x1005);

            //erase image1
            for(i=0 ; i < sectors; i++)
            {
                flash_erase(image1_head_faddr + i*FLASH_SECTOR_SIZE, Sector_Erase);
            }
            debug_show(0x1006);
        }
        else
        {
            debug_show(0x2000);
        }
    }
}


void main()
{
    GLOBAL_INT_STOP();
    NVIC_DisableIRQ(UART0_IRQn);
    srst_per(UART0_SRST_PER);
    zero_init();
    sysctrl_io_init();
    clk_gate_clr_all_clk();
    pshare_reset_to_gpio();
    app_dmac_init_wrapper();
    app_qspi_param_init_wrapper(&boot_ram_head.flash.multi_read_param);
    app_qspi_init_wrapper();
    cache_config();
    if(boot_ram_head.flash.multi_read_param.dual_quad == Quad_SPI_Format)
    {
        quad_enable();
    }
    cache_enable();

    #ifdef ENABLE_LOW_BATTERY_WAIT
    battery_sample_in();
    battery_sample_out();
    #endif

    //ota block move
    ota_process();
    
    #ifdef ENABLE_BOOTRAM_WDT
    //set wdt
    bootram_wdt_open();
    #endif

    //jump
    uint32_t *vec_int_base = (uint32_t *)(FLASH_MAPPED_ADDR + IMAGE_OFFSET_BASE);
    __set_MSP(*vec_int_base);
    typedef void (*void_fn)(app_info_t *,flash_info_t *,uint32_t,uint32_t);    
    void_fn *reset_handler = (void_fn *)(vec_int_base + 1);
    // Jump to user's Reset_Handler
    (*reset_handler)(&boot_ram_head.app,&boot_ram_head.flash,0,BOOTINIT_FLAG);    
}
