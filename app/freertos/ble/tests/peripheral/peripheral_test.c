#define LOG_TAG "spim_test"
#define LOG_LVL LVL_DBG

#include "bx_log.h"
#include "spi_test.h"
#include "app_uart.h"
#include "io_ctrl.h"

app_uart_inst_t uart0 = UART_INSTANCE(0);

static void uart_init()
{
    uart0.param.baud_rate = UART_BAUDRATE_115200;
    uart0.param.rx_pin_no = 13;
    uart0.param.tx_pin_no = 12;
    uart0.param.tx_dma = 1;
    uart0.param.rx_dma = 1;
    app_uart_init(&uart0.inst);
}

static void spim_callback(uint8_t stage,uint8_t status)
{
    
    
}

static void spis_callback(uint8_t stage,uint8_t status)
{
    
    
}

void user_init()
{
    uart_init(); //for result output
    spim1_init();
    spis_init();
    //TODO init
    
    
    spim_spis_test(spim_callback,spis_callback);
    //TODO test
}