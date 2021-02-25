#include <stdint.h>
#include "arch.h"
#include "rwip.h"
#include "arch_init.h"
#include "app_uart.h"
#include "gapm.h"
#define DEFAULT_HCI  0
extern uint32_t error;
app_uart_inst_t uart0 = UART_INSTANCE(0);


void uart0_read(uint8_t * bufptr, uint32_t size, void(* callback)(void *, uint8_t), void * dummy)
{
    periph_err_t stat = app_uart_read(&uart0.inst,bufptr,size,callback, dummy);
    BX_ASSERT(stat == PERIPH_NO_ERROR);
}

void uart0_write(uint8_t * bufptr, uint32_t size, void(* callback)(void *, uint8_t), void * dummy)
{
#ifdef METAL_APP_TWO_MCU_SLAVE
    two_mcu_slave_wakeup_master();
#endif
    app_uart_write(&uart0.inst,bufptr,size,callback, dummy);
}

void uart0_flow_on()
{
    app_uart_flow_on(&uart0.inst);
}

bool uart0_flow_off()
{
    return app_uart_flow_off(&uart0.inst);
}

const struct rwip_eif_api *serial_eif_get()
{
    static const struct rwip_eif_api uart0_eif =
    {
        uart0_read,
        uart0_write,
        uart0_flow_on,
        uart0_flow_off,
    };
    return &uart0_eif;
}

void ahi_uart_init()
{
    uart0.param.baud_rate = UART_BAUDRATE_9600;
    uart0.param.rx_pin_no = 13;
    uart0.param.tx_pin_no = 12;
    uart0.param.tx_dma = 1;
    uart0.param.rx_dma = 1;
    app_uart_init(&uart0.inst);
}

#include "reg_sysc_cpu.h"
#include "log.h"
int main()
{
    soc_initialize();
    uint32_t min=~0;
    uint32_t max=0;

    uint32_t i = 0;
    do
    {

        sysc_cpu_calb32k_start_setf(1);
        if(i==50000)
        {
            LOG(LOG_LVL_INFO,"min:%d,max%d\n",min,max);
            i = 0;
            min=~0;
            max=0;
        }
        while(!(sysc_cpu_calb32k_start_getf() == 0));
        uint32_t current = sysc_cpu_calb32k_rslt_getf();
        if(current<min)
        {
            min = current;
        }
        if(current>max)
        {
            max = current;
        }
        ++i;
    }while(1);


    /*
    ahi_uart_init();
    
    baremetal_blestack_init();
    #if (DEFAULT_HCI  == 1)
    gapm_set_embedded_host(false);
    #endif
    soc_baremetal_loop();
    */
}
