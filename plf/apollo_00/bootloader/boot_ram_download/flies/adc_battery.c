#include "stdbool.h"
#include "adc_battery.h"
#include "reg_adc.h"
#include "field_manipulate.h"
#include "_reg_base_addr.h"


#define ADC_DIFFERENCE_MAX      50

#define ADC_BATTERY_2V8         524
#define ADC_BATTERY_3V0         494
#define ADC_BATTERY_3V3         485
#define ADC_BATTERY_3V5         467
#define ADC_BATTERY_MAX         ADC_BATTERY_3V0



#define BX_DELAY_US(a) do\
                       {\
                            for(volatile uint32_t i=0;i<(uint32_t)((uint32_t)(a))*(32000000/4000000);i++); \
                       }while(0)

                       

uint32_t *p_adc_debug = (uint32_t*)0x128000;
                       
static uint16_t get_battery_adc_and_delay(void)
{
    reg_adc_t *reg = (reg_adc_t *)REG_ADC_CTRL_BASE;
    uint16_t value;
    //init
    FIELD_WR(reg, CTRL0, ADC_LDO_FORCE_ON, 1);
    FIELD_WR(reg, CTRL_SNGL, ADC_SNGL_CH   , 3);
    FIELD_WR(reg, CTRL_SNGL, ADC_SNGL_START, 1);
    // wait adc data ready.
    while(FIELD_RD(reg, CTRL_SNGL, ADC_SNGL_START));
    value  = FIELD_RD(reg, CTRL_SNGL, ADC_ADC_DATA_SNGL);
    //delay for next sample
    BX_DELAY_US(100*1000);
    //debug
    *p_adc_debug = value;
    p_adc_debug++;
    return value;
}

static uint16_t adc_calc_diff(uint16_t a , uint16_t b)
{
    if(a > b) return a-b;
    else      return b-a;
}

static uint16_t get_average_adc_val(void)
{
    static uint16_t adc_last = 0;
    uint16_t adc_now;
    //get data
    adc_now = get_battery_adc_and_delay();
    //wait until adc value smooth
    while(adc_calc_diff(adc_now , adc_last) > ADC_DIFFERENCE_MAX)
    {
        adc_last = adc_now;
        adc_now = get_battery_adc_and_delay();
    }
    return adc_now;
}

void battery_sample_in(void)
{
    //set battert monitor mode
    *(volatile uint32_t*)0x2020108c = 0x22d80;  
    //sample battery loop
    while(true)
    {
        if(get_average_adc_val() < ADC_BATTERY_MAX)
        {
            return;
        }
    }
}



void battery_sample_out(void)
{
    reg_adc_t *reg = (reg_adc_t *)REG_ADC_CTRL_BASE;
    //recover rf settings
    *(volatile uint32_t*)0x2020108c = 0;  
    //uninit adc
    FIELD_WR(reg, CTRL0, ADC_LDO_FORCE_ON, 0);
}

