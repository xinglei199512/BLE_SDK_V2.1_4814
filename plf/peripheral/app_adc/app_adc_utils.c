/*
 * app_adc_utils.c
 *
 *  Created on: 20180910
 *      Author: mingzhou
 */

#include "app_adc.h"
#include "plf.h"
#include <stdlib.h>
#include "log.h"
#include "rf_reg_settings.h"
#include "io_ctrl.h"
#include "flash_wrapper.h"
#include "app_adc_utils.h"
#include "rf_temp_adjust.h"

#if HW_BX_VERSION == 00

//#if (defined(CFG_FREERTOS_SUPPORT)&&(CFG_FREERTOS_SUPPORT==1))
//#include "FreeRTOS.h"
//#include "timers.h"
//#endif

#define ADC_SAMPLE_NUM 4
#define ADC_DEL_NUM 1
#define CP_SAMPLE_NUM ADC_SAMPLE_NUM
#define BAT_SAMPLE_NUM ADC_SAMPLE_NUM
#define TMP_SAMPLE_NUM ADC_SAMPLE_NUM
#define CP_DEL_NUM ADC_DEL_NUM

#define ADC_LDO_DELAY_DEFAULT      8
#define ADC_CAL_NUM 64
#define ADC_BAT_CAL_BASE_VAL 464
#define ADC_BATTERY_CHN 3
#define M_BAT_BASE              940

#define ADC_BAT_VAL_HIGH 512
#define ADC_BAT_VAL_LOW 342
#define ADC_BAT_HIGH 5000
#define ADC_BAT_LOW 3000
#define ADC_BAT_VOLT_STEP 25
#define ADC_BAT_MINUS_STEP 8
#define ADC_BAT_VAL_STEP (ADC_BAT_HIGH-ADC_BAT_LOW)

#define ADC_SINGLE_END_VAL_HIGH 1023000
#define ADC_SINGLE_END_VAL_LOW 0
#define ADC_SINGLE_END_HIGH 2850
#define ADC_SINGLE_END_LOW  0
#define ADC_SINGLE_END_VOLT_STEP 25
#define ADC_SINGLE_END_INDEX_HIGH ((ADC_SINGLE_END_HIGH - ADC_SINGLE_END_LOW)/ADC_SINGLE_END_VOLT_STEP)
#define ADC_SINGLE_END_SUBSTRATOR 6000
#define ADC_SINGLE_END_VAL_STEP 9

#if (defined BX_BATTERY_MONITOR) && (BX_BATTERY_MONITOR == 1)  
#define BAT_VOLT_ARRAY_SIZE 1
#if (defined(CFG_FREERTOS_SUPPORT)&&(CFG_FREERTOS_SUPPORT==1))
StaticTimer_t xTimerBuffers_Bat;
TimerHandle_t xTimerBat;
#endif
#endif

#if (defined BX_TEMP_SENSOR) && (BX_TEMP_SENSOR == 1)  
#define TEMP_SENSOR_ARRAY_SIZE 1
#if (defined(CFG_FREERTOS_SUPPORT)&&(CFG_FREERTOS_SUPPORT==1))
StaticTimer_t xTimerBuffers_Temp;
TimerHandle_t xTimerTemp;
#endif
#endif

//static uint8_t m_bat_3_offset[] = {345-ADC_BAT_VAL_LOW, 362-ADC_BAT_VAL_LOW, 381-ADC_BAT_VAL_LOW, 398-ADC_BAT_VAL_LOW, 415-ADC_BAT_VAL_LOW,
  //                                 434-ADC_BAT_VAL_LOW, 451-ADC_BAT_VAL_LOW, 470-ADC_BAT_VAL_LOW, 487-ADC_BAT_VAL_LOW, 504-ADC_BAT_VAL_LOW};
#if ADC_TEMP_SENSOR_EN
static uint8_t m_bat[ADC_CAL_NUM] =
{
    0,
    2,
    4,
    6,
    8,
    10,
    12,
    14,
    16,
    18,
    20,
    22,
    25,
    27,
    29,
    31,
    33,
    35,
    38,
    40,
    42,
    44,
    46,
    49,
    51,
    53,
    55,
    58,
    60,
    62,
    65,
    67,
    69,
    72,
    74,
    76,
    79,
    81,
    83,
    86,
    88,
    91,
    93,
    96,
    98,
    101,
    103,
    106,
    108,
    111,
    113,
    116,
    118,
    121,
    123,
    126,
    129,
    131,
    134,
    137,
    139,
    142,
    145,
    147
};
#endif

static uint16_t m_adc_2_bat_k[ADC_CAL_NUM] =
{
    12886-10667,
    12837-10667,
    12802-10667,
    12752-10667,
    12713-10667,
    12679-10667,
    12651-10667,
    12557-10667,
    12500-10667,
    12500-10667,
    12500-10667,
    12500-10667,
    12392-10667,
    12336-10667,
    12312-10667,
    12299-10667,
    12240-10667,
    12192-10667,
    12164-10667,
    12120-10667,
    12086-10667,
    12049-10667,
    12019-10667,
    11972-10667,
    11935-10667,
    11903-10667,
    11866-10667,
    11823-10667,
    11800-10667,
    11760-10667,
    11716-10667,
    11688-10667,
    11656-10667,
    11626-10667,
    11584-10667,
    11553-10667,
    11517-10667,
    11486-10667,
    11445-10667,
    11418-10667,
    11375-10667,
    11345-10667,
    11319-10667,
    11280-10667,
    11250-10667,
    11221-10667,
    11187-10667,
    11149-10667,
    11121-10667,
    11089-10667,
    11056-10667,
    11028-10667,
    10997-10667,
    10969-10667,
    10947-10667,
    10908-10667,
    10870-10667,
    10843-10667,
    10812-10667,
    10780-10667,
    10753-10667,
    10713-10667,
    10699-10667,
    10667-10667,
};

static uint32_t m_adc_2_bat_base[ADC_CAL_NUM] =
{
    6658000,
    6635500,
    6613600,
    6591400,
    6570600,
    6551300,
    6530700,
    6499900,
    6475000,
    6462500,
    6450000,
    6437500,
    6404700,
    6381100,
    6363700,
    6348700,
    6324300,
    6302600,
    6284700,
    6263800,
    6244000,
    6227800,
    6209200,
    6187300,
    6167400,
    6151600,
    6132300,
    6111400,
    6097100,
    6076700,
    6055700,
    6040700,
    6022000,
    6006700,
    5985700,
    5970400,
    5951000,
    5935000,
    5914700,
    5900100,
    5879300,
    5863800,
    5849400,
    5829400,
    5813800,
    5798700,
    5779400,
    5762900,
    5747800,
    5732400,
    5713200,
    5698300,
    5682600,
    5667800,
    5653300,
    5637300,
    5617000,
    5602300,
    5587000,
    5571400,
    5556600,
    5539100,
    5536900,
    5511100,
};
#if ADC_GPADC_SINGLE_END_MODE_EN
#define ADC_GPADC_BASE_VAL_B 904
static uint8_t m_adc_beta_offset_904[ADC_CAL_NUM] =
{
    187,
    187,
    181,
    174,
    174,
    168,
    168,
    161,
    155,
    155,
    149,
    149,
    149,
    143,
    137,
    137,
    131,
    131,
    131,
    119,
    119,
    113,
    113,
    113,
    102,
    102,
    102,
    96,
    96,
    96,
    85,
    85,
    85,
    80,
    80,
    74,
    69,
    69,
    64,
    64,
    58,
    53,
    53,
    53,
    48,
    48,
    43,
    38,
    38,
    38,
    33,
    28,
    23,
    23,
    23,
    19,
    14,
    14,
    9,
    9,
    9,
    5,
    0,
    0
};
#endif

#if ADC_GPADC_DIFFERENTIAL_MODE_EN
#define ADC_GPADC_DIF_BASE_VAL 920
static uint8_t m_adc_dif_offset_920[ADC_CAL_NUM] =
{
    0,
    2,
    4,
    7,
    9,
    11,
    14,
    15,
    18,
    21,
    23,
    25,
    27,
    30,
    31,
    34,
    36,
    39,
    41,
    43,
    45,
    48,
    50,
    53,
    55,
    58,
    59,
    61,
    64,
    66,
    68,
    71,
    73,
    75,
    78,
    80,
    82,
    85,
    87,
    89,
    91,
    94,
    96,
    98,
    100,
    102,
    106,
    108,
    110,
    112,
    114,
    116,
    120,
    122,
    124,
    126,
    128,
    130,
    133,
    135,
    137,
    139,
    142,
    144,
};
#endif

static uint8_t adc_bonding_RO_array[32] =
{
    [4] = 51,
    [5] = 48,
    [6] = 45,
    [7] = 42,
    [8] = 39,
    [9] = 36,
    [10] = 33,
    [11] = 30,
    [12] = 27,
    [13] = 24,
    [14] = 21,
};

int32_t adc_cp_RO = 0;

/**
 *******************************************************************************************************************************************
 * @brief          Calculate the average value of the array items excluding the maximum & minimum. Round in & round out will also be executed.
 * @param[inout]   *numbers    Pointer of array.
 * @return         Average value.
 *******************************************************************************************************************************************
 */
static uint16_t AvgArray(uint16_t* numbers);
/**
 *******************************************************************************************************************************************
 * @brief          ADC comparator function for qsort.
 * @param[in]      *a    Pointer of item to be compared.
 * @param[out]     *b    Pointer of item to be compared. 
 * @return         Compared result.
 *******************************************************************************************************************************************
 */
static int adc_comparator(const void *a, const void *b);

/**
 *******************************************************************************************************************************************
 * @brief          Function getting battery monitor raw value.
 * @param[in]      *adc_p    Pointer of adc instance.
 * @return         Battery raw value.
 *******************************************************************************************************************************************
 */
static uint16_t app_adc_battery_raw(app_adc_inst_t *adc_p);

/**
 *******************************************************************************************************************************************
 * @brief          Function getting gpadc single end raw value. 
 * @param[in]      *adc_p    Pointer of adc instance.
 * @return         Single end gpadc raw value.
 *******************************************************************************************************************************************
 */
static uint16_t app_adc_gpadc_single_end_raw(uint8_t channel);
/**
 *******************************************************************************************************************************************
 * @brief          Function judging gpadc bonding trim RO bits.
 * @param[in]      channel  ADC channel.
 * @return         Battery raw value.
 *******************************************************************************************************************************************
 */
static uint8_t app_adc_gpadc_bonding_RO_bit(uint32_t channel);

/**
 *******************************************************************************************************************************************
 * @brief          Convert battery adc value to voltage.
 * @param[in]      bat_val Battery value.
 * @return         Voltage value(in mini volt).
 *******************************************************************************************************************************************
 */
static uint32_t app_adc_battery_volt(uint32_t bat_val);

#if ADC_TEMP_SENSOR_EN
/**
 *******************************************************************************************************************************************
 * @brief          Function getting temperature sensor raw value.
 * @param[in]      *adc_p    Pointer of adc instance.
 * @return         Temperature sensor raw value.
 *******************************************************************************************************************************************
 */
static uint16_t app_adc_tempSensor_raw(app_adc_inst_t *adc_p);
#endif

#if ADC_GPADC_SINGLE_END_MODE_EN
/**
 *******************************************************************************************************************************************
 * @brief          Convert single end adc value to voltage.
 * @param[in]      gpadc_val Gpadc value.
 * @return         Voltage value(in mini volt).
 *******************************************************************************************************************************************
 */
static uint32_t app_adc_gpadc_single_end_volt(uint32_t gpadc_val);
#endif

static uint16_t AvgArray(uint16_t* numbers)
{
    uint32_t sum = 0;
    uint32_t avg = 0;
    for(uint32_t i = ADC_DEL_NUM; i <= ADC_SAMPLE_NUM-ADC_DEL_NUM-ADC_DEL_NUM; i++)
    {
        sum += numbers[i];
    }
    avg = sum/(ADC_SAMPLE_NUM-ADC_DEL_NUM-ADC_DEL_NUM);
    if(sum - avg*(ADC_SAMPLE_NUM-ADC_DEL_NUM-ADC_DEL_NUM) >= (ADC_SAMPLE_NUM-ADC_DEL_NUM-ADC_DEL_NUM)/2)
    {
        avg++;
    }
    return (uint16_t)avg;
}
#if (defined BX_BATTERY_MONITOR) && (BX_BATTERY_MONITOR == 1)  
static uint16_t bat_cur_volt[BAT_VOLT_ARRAY_SIZE] = {0};
void app_set_cur_bat(void)
{
    #if BAT_VOLT_ARRAY_SIZE > 1
    memcpy((void*)bat_cur_volt[0], (void*)bat_cur_volt[1], (BAT_VOLT_ARRAY_SIZE-1)*sizeof(bat_cur_volt[0]));
    bat_cur_volt[BAT_VOLT_ARRAY_SIZE-1] = (uint16_t)(app_adc_battery()/1000);
    if(bat_cur_volt[0] == 0) // first sample
    {
        for(uint32_t i = 0; i < BAT_VOLT_ARRAY_SIZE-1; i++)
        {
            bat_cur_volt[i] = bat_cur_volt[BAT_VOLT_ARRAY_SIZE-1];
        }
    }
    #else
    bat_cur_volt[0] = (uint16_t)(app_adc_battery()/1000);
    #endif
}
uint16_t app_get_cur_bat(void)
{
    uint32_t sum_bat = 0;
    for(uint32_t i = 0; i < BAT_VOLT_ARRAY_SIZE; i++)
    {
        sum_bat += bat_cur_volt[i];
    }
    return (uint16_t)(sum_bat/BAT_VOLT_ARRAY_SIZE);
}
#if (defined(CFG_FREERTOS_SUPPORT)&&(CFG_FREERTOS_SUPPORT==1))
void adc_sys_adjust_bat(TimerHandle_t xTimer)
{
    app_set_cur_bat();
    rf_reg_adjust_bat(app_get_cur_bat());
}
#endif
#endif
//static int adc_comparator(const void *a, const void *b);
/*
void app_adc_readRO(void)
{
    *(volatile uint32_t*)0x20201078 |= 0x3f<<7;
    *(volatile uint32_t*)0x2020107c |= 0x7<<23;

    BX_DELAY_US(10);
    adc_cp_RO = (*(volatile uint32_t*)0x202010b0 >> 8) & 0x3f;

    *(volatile uint32_t*)0x20201078 &= ~(0x3f<<7);
    *(volatile uint32_t*)0x2020107c &= ~(0x7<<23);
}
*/

static int adc_comparator(const void *a, const void *b)
{
    return *(uint16_t*)a > *(uint16_t*)b? 1: -1;
}

static uint16_t app_adc_battery_raw(app_adc_inst_t *adc_p)
{
    uint16_t result = 0;
    app_adc_inst_t adc = *adc_p;

//    *(volatile uint32_t*)0x2020108c = 0x22d80;
    rf_setting_battery_monitor_adc();
    if(app_adc_read_without_dma(&(adc.inst) , 3, &result))
    {
        result = (uint16_t)-1;
    }
    return result;
}

void app_adc_RO_sim_3V9(void)
{
    uint16_t adc_val[CP_SAMPLE_NUM];
    app_adc_inst_t adc = ADC_INSTANCE(0);
    //init
    adc.param.ldo_force_on = 1;
    adc.param.ldo_delay_us = ADC_LDO_DELAY_DEFAULT;
    adc.param.use_dma = 0;

    app_adc_init(&(adc.inst));

    for(uint8_t i = 0; i < CP_SAMPLE_NUM; i++)
    {
        adc_val[i] = app_adc_battery_raw(&adc);
        if(adc_val[i] == (uint16_t)-1)
        {
            i--;
            continue; // redo the sample
        }
    }
    app_adc_uninit(&(adc.inst));
    qsort(adc_val, CP_SAMPLE_NUM, sizeof(adc_val[0]), adc_comparator);
    adc_cp_RO = ADC_BAT_CAL_BASE_VAL - AvgArray(adc_val);
    if(adc_cp_RO > ADC_CAL_NUM-1)
    {
        adc_cp_RO = ADC_CAL_NUM-1;
    }
    if(adc_cp_RO < 0)
    {
        adc_cp_RO = 0;
    }
}

void app_adc_util_init(void)
{
    #ifdef ADC_RO_READ_FORCE_ON
    if(flash_read_security_reg(1, 0, 1, (uint8_t*)&adc_cp_RO) != PERIPH_NO_ERROR)
    {
        BX_ASSERT(0);
    }
    if(0xff == (uint8_t)adc_cp_RO)
    {
        uint8_t temp_bonding = 0;
    
        for(uint8_t i = 1; i < 6; i++)
        {
            temp_bonding |= app_adc_gpadc_bonding_RO_bit(i) << (i-1);
        }
        adc_cp_RO = adc_bonding_RO_array[temp_bonding];
    }
    #else
    adc_cp_RO = 33;
    #endif

    #if RF_PARAM == 2 || RF_PARAM == 3
    extern void refresh_rf_param_with_ro(uint32_t ro);
    refresh_rf_param_with_ro(adc_cp_RO);
    #endif

    #if (defined BX_BATTERY_MONITOR) && (BX_BATTERY_MONITOR == 1)
    #if (defined(CFG_FREERTOS_SUPPORT)&&(CFG_FREERTOS_SUPPORT==1))
    xTimerBat = xTimerCreateStatic( "BatTimer", pdMS_TO_TICKS(APP_BAT_TIMER_MS), pdTRUE, ( void * )0, adc_sys_adjust_bat, &xTimerBuffers_Bat);
    xTimerStart(xTimerBat, pdFALSE);
    #endif    
    #endif

    #if (defined BX_TEMP_SENSOR) && (BX_TEMP_SENSOR == 1)
    #if (defined(CFG_FREERTOS_SUPPORT)&&(CFG_FREERTOS_SUPPORT==1))
    xTimerTemp = xTimerCreateStatic( "TempTimer", pdMS_TO_TICKS(APP_TEMP_TIMER_MS), pdTRUE, ( void * )0, adc_sys_adjust_temp, &xTimerBuffers_Temp);
    xTimerStart(xTimerTemp, pdFALSE);
    #endif    
    #endif
  
    LOG(LOG_LVL_INFO,"RO=0x%x\n", adc_cp_RO);
}

static uint32_t app_adc_battery_volt(uint32_t bat_val)
{
    uint32_t k = m_adc_2_bat_k[adc_cp_RO] + 10667;    
    uint32_t base = m_adc_2_bat_base[adc_cp_RO];
    uint32_t adc_minus_320_mult_1000 = bat_val - 250;
    uint32_t adc_minus_320 = adc_minus_320_mult_1000;//adc_minus_320_mult_1000 >> 10;

    // LOG(LOG_LVL_INFO,"bat_val=%d, recovered adc=%d, RO=0x%x\n", bat_val, adc_minus_320, adc_cp_RO);
    return base - k*adc_minus_320;
}

uint32_t app_adc_battery(void)
{
    uint16_t adc_val[BAT_SAMPLE_NUM];
    uint16_t adc_val_16;

    app_adc_inst_t adc = ADC_INSTANCE(0);
    //init
    adc.param.ldo_force_on = 1;
    adc.param.ldo_delay_us = ADC_LDO_DELAY_DEFAULT;
    adc.param.use_dma = 0;

    app_adc_init(&(adc.inst));

    for(uint8_t i = 0; i < BAT_SAMPLE_NUM; i++)
    {
        adc_val[i] = app_adc_battery_raw(&adc);
        if(adc_val[i] == (uint16_t)-1)
        {
            i--; // redo the sample
            continue;
        }
    }
    app_adc_uninit(&(adc.inst));
    qsort(adc_val, BAT_SAMPLE_NUM, sizeof(adc_val[0]), adc_comparator);
    adc_val_16 = AvgArray(adc_val);
    // LOG(LOG_LVL_INFO,"bat raw value=%d, %d\n", adc_val[1], adc_val[16]);
    return app_adc_battery_volt(adc_val_16);
}

static uint16_t app_adc_gpadc_single_end_raw(uint8_t channel)
{
    uint16_t adc_val[8];
    //int32_t adc_val_cal = 0, adc_val_return = 0;
    int32_t adc_val_cal = 0;

    app_adc_inst_t adc = ADC_INSTANCE(0);
    //init
    adc.param.ldo_force_on = 1;
    adc.param.ldo_delay_us = ADC_LDO_DELAY_DEFAULT;
    adc.param.use_dma = 0;

    app_adc_init(&(adc.inst));

//    *(volatile uint32_t*)0x2020108c = 0x3980;
    rf_setting_single_mode_adc();

    for(uint8_t i = 0; i < 8; i++)
    {
        if(app_adc_read_without_dma(&(adc.inst) , channel, &adc_val[i]))
        {
            i--;
            continue; // redo the sample
        }
        adc_val_cal += adc_val[i];
    }
    //uninit
    app_adc_uninit(&(adc.inst));

    return (uint16_t)(adc_val_cal/8);
}

static uint8_t app_adc_gpadc_bonding_RO_bit(uint32_t channel)
{
    return app_adc_gpadc_single_end_raw(channel) < 675? 1: 0;
}

#if ADC_GPADC_SINGLE_END_MODE_EN
static uint32_t app_adc_gpadc_single_end_volt(uint32_t gpadc_val)
{
    uint32_t temp, quotient, reminder;
    if(gpadc_val > ADC_SINGLE_END_VAL_HIGH)
    {
        return ADC_SINGLE_END_LOW;
    }
    if(gpadc_val <= ADC_SINGLE_END_SUBSTRATOR) // not ADC_SINGLE_END_VAL_LOW
    {
        return ADC_SINGLE_END_HIGH;
    }

    temp = (gpadc_val - ADC_SINGLE_END_SUBSTRATOR)/ADC_SINGLE_END_VAL_STEP;
    quotient = temp/1000;
    reminder = temp%1000;

    if(reminder > 400) // round up and round down
    {
        quotient++;
    }

    return (ADC_SINGLE_END_INDEX_HIGH - 1 - quotient)*ADC_SINGLE_END_VOLT_STEP;
}


uint32_t adc_value_convert(uint16_t val_1 , uint16_t val_2)
{
    uint16_t adc_beta;
    int32_t adc_val_cal = 0, adc_val_return = 0;
    
    adc_beta = m_adc_beta_offset_904[adc_cp_RO] + ADC_GPADC_BASE_VAL_B;
    
    adc_val_cal += adc_beta*val_1 + 766000 - adc_beta*766;
    adc_val_cal += adc_beta*val_2 + 766000 - adc_beta*766;
    adc_val_cal*=2;
    
    adc_val_return = ((adc_val_cal-255000*4))/2;
    
    return app_adc_gpadc_single_end_volt((adc_val_return > 0? (uint32_t)adc_val_return: 0));
}


uint32_t app_adc_gpadc_single_end(uint8_t channel)
{
    uint16_t adc_val[4];
    uint16_t adc_beta;
    int32_t adc_val_cal = 0, adc_val_return = 0;

    app_adc_inst_t adc = ADC_INSTANCE(0);
    //init
    adc.param.ldo_force_on = 1;
    adc.param.ldo_delay_us = ADC_LDO_DELAY_DEFAULT;
    adc.param.use_dma = 0;

    app_adc_init(&(adc.inst));

//    *(volatile uint32_t*)0x2020108c = 0x3980;
    rf_setting_single_mode_adc();

    adc_beta = m_adc_beta_offset_904[adc_cp_RO] + ADC_GPADC_BASE_VAL_B;

    for(uint8_t i = 0; i < 4; i++)
    {
        if(app_adc_read_without_dma(&(adc.inst) , channel, &adc_val[i]))
        {
            i--;
            continue; // redo the sample
        }
        adc_val_cal += adc_beta*adc_val[i] + 766000 - adc_beta*766;
    }
    //uninit
    app_adc_uninit(&(adc.inst));

    adc_val_return = ((adc_val_cal-255000*4))/2;  // adc/2 = adc*2/4

    return app_adc_gpadc_single_end_volt((adc_val_return > 0? (uint32_t)adc_val_return: 0));
}
#endif

#if ADC_TEMP_SENSOR_EN
static uint16_t app_adc_tempSensor_raw(app_adc_inst_t *adc_p)
{
    uint16_t result = 0;
    app_adc_inst_t adc = *adc_p;

//    *(volatile uint32_t*)0x2020108c = 0x12580;
    rf_setting_temperature_adc();
    if(app_adc_read_without_dma(&(adc.inst) , 3, &result))
    {
        result = (uint16_t)-1;
    }
    return result;
}

int16_t app_adc_tempSensor(void)
{
    uint16_t adc_val[TMP_SAMPLE_NUM];
    int32_t adc_val_cal;
    uint16_t adc_val_16;
    int16_t temp_val;

    app_adc_inst_t adc = ADC_INSTANCE(0);
    
    // init
    adc.param.ldo_force_on = 1;
    adc.param.ldo_delay_us = ADC_LDO_DELAY_DEFAULT;
    adc.param.use_dma = 0;

    app_adc_init(&(adc.inst));

    for(uint8_t i = 0; i < TMP_SAMPLE_NUM; i++)
    {
        adc_val[i] = app_adc_tempSensor_raw(&adc);
        if(adc_val[i] == (uint16_t)-1)
        {
            i--;
            continue; // redo the sample
        }
    }
    
    app_adc_uninit(&(adc.inst));
    qsort(adc_val, TMP_SAMPLE_NUM, sizeof(adc_val[0]), adc_comparator);
    adc_val_16 = AvgArray(adc_val);
    adc_val_cal = (int32_t)adc_val_16*(m_bat[adc_cp_RO] + M_BAT_BASE);
    temp_val = (adc_val_cal - 448230)/720;
//    LOG(LOG_LVL_INFO,"RO=%d\n", adc_cp_RO);
//    LOG(LOG_LVL_INFO,"adc temp raw value=%d, temp=%d\n", adc_val_16, temp_val);
    return temp_val;
}
#endif

#if (defined BX_TEMP_SENSOR) && (BX_TEMP_SENSOR == 1)  
static int16_t temp_sensor[TEMP_SENSOR_ARRAY_SIZE] = {0xFF};
void app_set_cur_temp(void)
{
    #if TEMP_SENSOR_ARRAY_SIZE > 1
    memcpy((void*)temp_sensor[0], (void*)temp_sensor[1], (TEMP_SENSOR_ARRAY_SIZE-1)*sizeof(temp_sensor[0]));
    temp_sensor[TEMP_SENSOR_ARRAY_SIZE-1] = app_adc_tempSensor()/1000;
    if(temp_sensor[0] == 0xFF) // first sample
    {
        for(uint32_t i = 0; i < TEMP_SENSOR_ARRAY_SIZE-1; i++)
        {
            temp_sensor[i] = temp_sensor[TEMP_SENSOR_ARRAY_SIZE-1];
        }
    }
    #else
    temp_sensor[0] = app_adc_tempSensor()/1000;
    #endif
}
int16_t app_get_cur_temp(void)
{
    int32_t sum_temp = 0;
    for(uint32_t i = 0; i < TEMP_SENSOR_ARRAY_SIZE; i++)
    {
        sum_temp += temp_sensor[i];
    }
    return (int16_t)(sum_temp/TEMP_SENSOR_ARRAY_SIZE);
}
#if (defined(CFG_FREERTOS_SUPPORT)&&(CFG_FREERTOS_SUPPORT==1))
void adc_sys_adjust_temp(TimerHandle_t xTimer)
{
    //app_set_cur_temp();
    //rf_reg_adjust_temp(app_get_cur_temp());
	handle_read_temp();
}
#endif
#endif

#if ADC_GPADC_DIFFERENTIAL_MODE_EN
uint32_t app_adc_gpadc_differential(uint8_t channel)
{
    uint16_t adc_val;
    uint16_t adc_beta;
    uint32_t adc_val_cal = 0;

    app_adc_inst_t adc = ADC_INSTANCE(0);
    //init
    adc.param.ldo_force_on = 1;
    adc.param.ldo_delay_us = ADC_LDO_DELAY_DEFAULT;
    adc.param.use_dma = 0;

    app_adc_init(&(adc.inst));

//    *(volatile uint32_t*)0x2020108c = 0x1980;
    rf_setting_differential_mode_adc();

    adc_beta = m_adc_dif_offset_920[adc_cp_RO] + ADC_GPADC_DIF_BASE_VAL;

    for(uint8_t i = 0; i < 16; i++)
    {
        if(app_adc_read_without_dma(&(adc.inst) , channel, &adc_val))
        {
            i--;
            continue; // redo the sample
        }
        adc_val_cal += adc_beta*adc_val;
    }
    //uninit
    app_adc_uninit(&(adc.inst));

    return (adc_val_cal)/4;
}
#endif

#elif HW_BX_VERSION == 01

#endif

