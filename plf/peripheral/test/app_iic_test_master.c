/*
 * app_iic_test.c
 *
 *  Created on: 2018-6-25
 *      Author: jiachuang
 */

#include "app_iic.h"
#include "string.h"
#include "plf.h"
#include "clk_gate.h"// use 32K RC calibration result as seed
#include "LOG.h"
#include "io_ctrl.h"
#include "rwip.h"
#include "ll.h"

//#define DEBUG_ERROR_DEVADDR
//AT24C02 device parameter (const)
#define AT24C16_DEV_ADDR	0xA0
#define AT24C16_PAGE_SIZE	16
#define AT24C16_SIZE		2048
//once transmit size
#define AT24C16_WRITE_SIZE  46      //  <=AT24C16_PAGE_SIZE
#define SLAVE_TEST_SIZE     30      //  slave once transmit size
//IIC test buffer
uint8_t iic_tx_buf[SLAVE_TEST_SIZE];
uint8_t iic_rx_buf[SLAVE_TEST_SIZE];
//CONST VAL
#define SLAVE_ADDR 0x30
//PIN DEFINE
#define APP_IIC0_TEST_SCL_PIN     8
#define APP_IIC0_TEST_SDA_PIN     9
#define APP_IIC1_TEST_SCL_PIN     10
#define APP_IIC1_TEST_SDA_PIN     11
#define APP_IIC_DEBUG_PIN         15

//debug parameter
__IO uint8_t iic_tx_ok = 0;
__IO uint8_t iic_rx_ok = 0;
__IO uint8_t iic_master_tx_ok = 0;
__IO uint8_t iic_master_rx_ok = 0;
__IO uint8_t iic_slave_tx_ok = 0;
__IO uint8_t iic_slave_rx_ok = 0;

//instance
app_iic_inst_t iic0 = IIC_INSTANCE(0);
app_iic_inst_t iic1 = IIC_INSTANCE(1);

static void app_iic_delay_ms(uint32_t ms)
{
    while(ms --)
    {
        BX_DELAY_US(1000);
    }
}
static void toggle_debug(void)
{
    io_pin_write(APP_IIC_DEBUG_PIN,1);
    BX_DELAY_US(1);
    io_pin_write(APP_IIC_DEBUG_PIN,0);
    BX_DELAY_US(1);
}

/***********************************START MASTER TEST************************************/
static void app_iic_tx_finish(void* dummy , uint8_t stat)
{
	iic_tx_ok = 1;
    //debug
    toggle_debug();
}
static void app_iic_rx_finish(void* dummy , uint8_t stat)
{
	iic_rx_ok = 1;
    //debug
    toggle_debug();
}
static uint8_t app_iic_eeprom_test(app_iic_inst_t *handle , uint8_t use_dma , uint8_t speed_mode)
{
    //init parameter
    handle->param.dev_addr_bit_num = IIC_7BIT_ADDRESS;
    handle->param.enable_pull_up   = true;
    handle->param.mem_addr_bit_num = IIC_8BIT_MEMORY_ADDRESS;
    handle->param.scl_pin          = APP_IIC0_TEST_SCL_PIN;
    handle->param.sda_pin          = APP_IIC0_TEST_SDA_PIN;
    handle->param.slave_address    = 0;
    handle->param.work_mode        = IIC_MASTER;
    handle->param.speed_mode       = (app_iic_speed_mode_t)speed_mode;
    handle->param.use_dma          = use_dma;
    app_iic_init(&handle->inst);

	uint16_t page_index , length , device_address , mem_address , i , success = 1;
	use_dma = handle->param.use_dma;
    uint32_t err_r,err_w;
    
	//verify
	//for(page_index = 0;page_index < AT24C16_SIZE / AT24C16_PAGE_SIZE ; page_index ++)
    for(page_index = 0;page_index < 5 ; page_index ++)
    for(length = 1;length <= AT24C16_WRITE_SIZE;length ++)
	{
		//prepare data
        //page_index = 3;
		device_address = AT24C16_DEV_ADDR + (((page_index * AT24C16_PAGE_SIZE) >> 8) << 1);
        
		mem_address    = (page_index * AT24C16_PAGE_SIZE) & 0xFF;
	    for(i = 0;i < AT24C16_WRITE_SIZE; i++) {iic_tx_buf[i] = co_rand_byte();}
		iic_tx_ok = iic_rx_ok = 0;

		//write
        //LOG(3,"\nW%d:\n",length);
        err_w = bx_iic_write(&handle->inst , iic_tx_buf , length , device_address , mem_address , 100);
		toggle_debug();
        app_iic_delay_ms(5);
		//read
        //LOG(3,"\nR%d:\n",length);
        err_r = bx_iic_read(&handle->inst , iic_rx_buf , length , device_address , mem_address , 100);
        toggle_debug();
		app_iic_delay_ms(1);

		//verify
        if((err_r!=0) || (err_w!=0)) LOG(LOG_LVL_INFO , "iic rw error!!%d,%d\n",err_r,err_w);//error handle
		if(memcmp(iic_tx_buf,iic_rx_buf,length) != 0)
		{
			LOG(LOG_LVL_INFO,"length=%d,ERROR\n",length);
			success = 0;
		}
		else
		{
            //LOG(LOG_LVL_INFO,"length=%d,OK\n",length);
            LOG(LOG_LVL_INFO,".");
		}
	}
	app_iic_uninit(&handle->inst);
	return success;
}


static void app_iic_master_test(void)
{
    co_random_init(666);
    for(char use_dma = 0;use_dma <=0 ; use_dma ++)
    {
    	for(char speed = 1;speed <= 1 ; speed ++)
    	{
    		LOG(3,"MASTER:USE_DMA=%d,SPEED=%d\n",use_dma,speed);
    		if(app_iic_eeprom_test(&iic0 , use_dma , speed) == 0)	{LOG(LOG_LVL_INFO,"IIC0 ERROR\n");}
    		if(app_iic_eeprom_test(&iic1 , use_dma , speed) == 0)	{LOG(LOG_LVL_INFO,"IIC1 ERROR\n");}
    	}
    }
}


/***********************************START SLAVE TEST************************************/

static void app_iic_master_tx_finish(void* ptr , uint8_t dummy)
{
	iic_master_tx_ok = 1;
    //debug
    toggle_debug();
}
static void app_iic_master_rx_finish(void* ptr , uint8_t dummy)
{
	iic_master_rx_ok = 1;
    //debug
    toggle_debug();
}
static void app_iic_slave_tx_finish(void* ptr , uint8_t dummy)
{
	iic_slave_tx_ok = 1;
    //debug
    toggle_debug();
}
static void app_iic_slave_rx_finish(void* ptr , uint8_t dummy)
{
	iic_slave_rx_ok = 1;
    //debug
    toggle_debug();
}

static uint32_t app_iic_slave_module_test(app_iic_inst_t *hdl_master , app_iic_inst_t *hdl_slave , uint8_t speed_mode)
{
    uint32_t i , curr_error = 0;
    periph_err_t err_r,err_w;
    //iic0 global parameter
    iic0.param.dev_addr_bit_num = IIC_7BIT_ADDRESS;
    iic0.param.enable_pull_up   = true;
    iic0.param.mem_addr_bit_num = IIC_NO_MEMORY_ADDRESS;
    iic0.param.scl_pin          = APP_IIC0_TEST_SCL_PIN;
    iic0.param.sda_pin          = APP_IIC0_TEST_SDA_PIN;
    iic0.param.slave_address    = SLAVE_ADDR;
    iic0.param.speed_mode       = (app_iic_speed_mode_t)speed_mode;
    iic0.param.use_dma          = 0;
    //iic1 global parameter
    iic1.param = iic0.param;
    iic1.param.scl_pin          = APP_IIC1_TEST_SCL_PIN;
    iic1.param.sda_pin          = APP_IIC1_TEST_SDA_PIN;
    //init private parameter
    hdl_master->param.work_mode        = IIC_MASTER;
    hdl_slave ->param.work_mode        = IIC_SLAVE;

    //Master send slave receive
    app_iic_init(&hdl_master->inst);
    app_iic_init(&hdl_slave ->inst);
    for(i = 0;i < SLAVE_TEST_SIZE; i++) {iic_tx_buf[i] = co_rand_byte();}
    memset(iic_rx_buf,0,sizeof(iic_rx_buf));
    iic_master_tx_ok = iic_master_rx_ok = iic_slave_tx_ok = iic_slave_rx_ok = 0;
    err_r = app_iic_read (&hdl_slave ->inst , iic_rx_buf , SLAVE_TEST_SIZE , SLAVE_ADDR , 0 , app_iic_slave_rx_finish  , 0);//s
    err_w = app_iic_write(&hdl_master->inst , iic_tx_buf , SLAVE_TEST_SIZE , SLAVE_ADDR , 0 , app_iic_master_tx_finish , 0);//m
    while(iic_master_tx_ok == 0);
    while(iic_slave_rx_ok == 0);
    app_iic_delay_ms(20);
    //verify
    if(memcmp(iic_tx_buf,iic_rx_buf,SLAVE_TEST_SIZE) != 0) curr_error = 1;
    if((err_r!=PERIPH_NO_ERROR) || (err_w!=PERIPH_NO_ERROR)) LOG(LOG_LVL_INFO , "iic rw error!!\n");//error handle
    app_iic_uninit(&hdl_slave ->inst);
    app_iic_uninit(&hdl_master->inst);    
    
    //Master read slave send
    app_iic_init(&hdl_master->inst);
    app_iic_init(&hdl_slave ->inst);
    for(i = 0;i < SLAVE_TEST_SIZE; i++) {iic_tx_buf[i] = co_rand_byte();}
    memset(iic_rx_buf,0,sizeof(iic_rx_buf));
    iic_master_tx_ok = iic_master_rx_ok = iic_slave_tx_ok = iic_slave_rx_ok = 0;
    err_w = app_iic_write(&hdl_slave ->inst , iic_tx_buf , SLAVE_TEST_SIZE , SLAVE_ADDR , 0 , app_iic_slave_tx_finish  , 0);//s
    err_r = app_iic_read (&hdl_master->inst , iic_rx_buf , SLAVE_TEST_SIZE , SLAVE_ADDR , 0 , app_iic_master_rx_finish , 0);//m
    while(iic_master_rx_ok == 0);
    while(iic_slave_tx_ok == 0);
    app_iic_delay_ms(20);
    //verify
    if(memcmp(iic_tx_buf,iic_rx_buf,SLAVE_TEST_SIZE) != 0)  curr_error = 1;
    if((err_r!=PERIPH_NO_ERROR) || (err_w!=PERIPH_NO_ERROR)) LOG(LOG_LVL_INFO , "iic rw error!!\n");//error handle
    app_iic_uninit(&hdl_slave ->inst);
    app_iic_uninit(&hdl_master->inst);    
    
    //return
    return curr_error;
}


static void app_iic_slave_test(void)
{
    app_iic_inst_t *hdl_master , *hdl_slave;
    co_random_init(777);
    for(char mode = 1;mode <= 2 ; mode ++)
    {
    	for(char speed = 1;speed <= 3 ; speed ++)
    	{
    		LOG(3,"SLAVE:MODE=%d,SPEED=%d:",mode,speed);
    		if(mode == 1) {hdl_master = &iic0 ; hdl_slave = &iic1;}
    		if(mode == 2) {hdl_master = &iic1 ; hdl_slave = &iic0;}
    		if(app_iic_slave_module_test(hdl_master , hdl_slave , speed) == 1)	{LOG(LOG_LVL_INFO,"SLAVE ERROR\n");}
    		else                                                                {LOG(LOG_LVL_INFO,"OK\n");}
    	}
    }
}



/***********************************SLAVE+MASTER TEST************************************/
void app_iic_s_m_test(void)
{
    __enable_irq();
	io_cfg_output(APP_IIC_DEBUG_PIN);
    while(*(__IO uint32_t *)4 != 0)
    {
        app_iic_master_test();
        //app_iic_slave_test();
    }
}





