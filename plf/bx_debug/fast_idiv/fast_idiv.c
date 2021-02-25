/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "log.h"
#include "io_ctrl.h"
#include "plf.h"
#include "co_math.h"


/*
 * EXPORT FUNCTION DEFINITIONS
 ****************************************************************************************
 */
//extern uint32_t DEBUG_IDIV(int32_t big , int32_t little);


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
volatile int32_t aa,bb,sys_div,noloop_div,sys_div_l,noloop_div_l;   //idiv_speed_test
int32_t times,a,b,c,d;  //div_stable_test


/**
 ****************************************************************************************
 * @brief   Test system div and fast_noloop_div speed
 ****************************************************************************************
 */
void idiv_speed_test(void)
{
    //init gpio
    io_cfg_output(13);
    BX_DELAY_US(100);
    
    aa=0x12345678;
    bb=0x23;
    
    io_pin_set(13);
    sys_div         = aa/bb;
    io_pin_clear(13);
    BX_DELAY_US(100);
    
    //io_pin_set(13);
    //noloop_div      = DEBUG_IDIV(aa,bb);
    //io_pin_clear(13);
    //BX_DELAY_US(100);
    
    io_pin_set(13);
    sys_div_l       = bb/aa;
    io_pin_clear(13);
    BX_DELAY_US(100);
    
    //io_pin_set(13);
    //noloop_div_l    = DEBUG_IDIV(bb,aa);
    //io_pin_clear(13);
    //BX_DELAY_US(100);

    //print result
    LOG(3,"sys_div=      %d\n",  sys_div);
    //LOG(3,"noloop_div=   %d\n",  noloop_div);
    LOG(3,"sys_div_l=    %d\n",  sys_div_l);
    //LOG(3,"noloop_div_l= %d\n",  noloop_div_l);

}


/**
 ****************************************************************************************
 * @brief   Test fast_noloop_div stability.
 ****************************************************************************************
 */
void div_stable_test(void)
{
    while(*(volatile uint32_t *)4 != 0)
    {
        times ++;
        a=-rand();
        b=rand()%0xFF;
        c=a/b;
        //d=DEBUG_IDIV(a,b);
        if(c != d)
        {
            LOG(3,"ERROR!!! a=0x%x,b=0x%x,c=0x%x,d=0x%x\n",a,b,c,d);
            BX_ASSERT(0);
        }
        
        if(times % 10000 == 0)
        {
            LOG(3,"times=%d\n",times);
        }
    }
}



/*
TEST RESULT:
items                 time          value
system uidiv          13.32us       0x12345678 / 0x23
noloop uidiv          10.25us       0x12345678 / 0x23
system uidiv(r0<r1)   11.88us       0x23 / 0x12345678
noloop uidiv(r0<r1)   1.63us        0x23 / 0x12345678

*/


