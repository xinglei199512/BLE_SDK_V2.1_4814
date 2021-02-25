#include "true_random.h"
#include "rc_calib.h"
#include "stdint.h"
#include "string.h"
#include "plf.h"
#include "reg_sysc_awo.h"
#include "reg_sysc_cpu.h"
#include "clk_gate.h"


/**
 ****************************************************************************************
 * @brief   get 32K cal value lsb 8bit
 ****************************************************************************************
 */

uint32_t   rand_seed;

uint32_t get_random_seed(void)
{
    return rand_seed;
}

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


/*
static void start_32k_xtal(void)
{
    sysc_awo_o_ana_32k_selxtal_setf(1); //start switching to 32k xtal
    BX_DELAY_US(150);
    sysc_awo_o_32k_rcosc_en_setf(0); // disable 32k rc
}

static void start_32k_rc(void)
{
    sysc_awo_o_32k_rcosc_en_setf(1); //enable 32k rcosc
    BX_DELAY_US(60*1000);
    sysc_awo_o_ana_32k_selxtal_setf(0); //start switching to 32k rcosc
    BX_DELAY_US(150);
}
*/


void hw_trng_get_numbers(uint8_t *buffer, uint8_t length)
{
    //start prepare
//    start_32k_rc();
    //get number
    for(uint8_t i=0;i<length;i++)
    {
        uint8_t tmp = 0;
        tmp |= get_32k_val_8bit() & 0x0F;
        tmp <<=4;
        tmp |= get_32k_val_8bit() & 0x0F;
        buffer[i] = tmp;
    }
    //end prepare
 //   start_32k_xtal();
}


void generate_random_seed(void)
{
    hw_trng_get_numbers((uint8_t *)&rand_seed, sizeof(rand_seed));
}




