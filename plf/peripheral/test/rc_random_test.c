/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rc_calib.h"
#include "app_uart.h"
#include "stdint.h"
#include "string.h"
#include "app_uart.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define RANDOM_ADDR_LENGTH  6
#define RANDOM_RC_CAL_NUM   8

/*
 * EXPORT VARIABLE DEFINITIONS
 ****************************************************************************************
 */
extern app_uart_inst_t uart0;

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct
{
    //use volatile to avoid compiler optimize
    volatile uint64_t rc0 : 6,
                      rc1 : 6,
                      rc2 : 6,
                      rc3 : 6,
                      rc4 : 6,
                      rc5 : 6,
                      rc6 : 6,
                      rc7 : 6,
                      reverse1 : 8,
                      reverse2 : 8;
}rc_random_val_t;

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
const uint8_t hex_tab[] = "0123456789ABCDEF";           //hex table for serial output
uint8_t  random_addr[RANDOM_ADDR_LENGTH];               //bd address
uint8_t  random_serial_send[RANDOM_ADDR_LENGTH*2+2];    //serial send string debug
volatile uint8_t serial_complete = 0;                   //serial complete
rc_random_val_t rc_random_val;                          //save temp rc random value



/**
 ****************************************************************************************
 * @brief   get 32K cal value lsb 8bit
 ****************************************************************************************
 */
static uint8_t get_32k_val_8bit()
{
    uint32_t cal_val=0;
    uint8_t  retval = 0;
    //start cal
    rc_calib_start();
    rc_calib_end(true);
    cal_val = get_rc32k_calib_val();
    //return
    retval = cal_val & 0xFF;
    return retval;
}

/**
 ****************************************************************************************
 * @brief   serial callback
 ****************************************************************************************
 */
static void serial_complete_callback(void* a,uint8_t b)
{
    serial_complete = 1;
}


/**
 ****************************************************************************************
 * @brief   make string and send from serial
 ****************************************************************************************
 */
static void serial_send(void)
{
    uint8_t i=0;
    uint8_t tmp_l,tmp_h;
    //make string buffer
    for(i=0;i<RANDOM_ADDR_LENGTH;i++)
    {
        tmp_h = random_addr[i]>>4;
        tmp_l = random_addr[i]&0x0F;
        random_serial_send[i*2+0] = hex_tab[tmp_h];
        random_serial_send[i*2+1] = hex_tab[tmp_l];
    }
    random_serial_send[RANDOM_ADDR_LENGTH*2+0] = '\r';
    random_serial_send[RANDOM_ADDR_LENGTH*2+1] = '\n';
    //send
    serial_complete = 0;
    app_uart_write(&uart0.inst , random_serial_send , sizeof(random_serial_send) , serial_complete_callback , 0);
    while(serial_complete == 0);
}

/**
 ****************************************************************************************
 * @brief   get random bd_addr
 ****************************************************************************************
 */
static void get_random_bd_addr(void)
{
    rc_random_val.rc0 = get_32k_val_8bit();
    rc_random_val.rc1 = get_32k_val_8bit();
    rc_random_val.rc2 = get_32k_val_8bit();
    rc_random_val.rc3 = get_32k_val_8bit();
    rc_random_val.rc4 = get_32k_val_8bit();
    rc_random_val.rc5 = get_32k_val_8bit();
    rc_random_val.rc6 = get_32k_val_8bit();
    rc_random_val.rc7 = get_32k_val_8bit();
    memcpy(random_addr , &rc_random_val , RANDOM_ADDR_LENGTH);
}

/**
 ****************************************************************************************
 * @brief   random test send to serial port.
 ****************************************************************************************
 */
void random_test(void)
{
    while(*(volatile uint32_t *)4 != 0)
    {
        //get random value
        get_random_bd_addr();
        //serial send random
        serial_send();
    }
}
