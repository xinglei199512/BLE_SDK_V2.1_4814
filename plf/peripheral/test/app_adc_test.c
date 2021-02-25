/*
 * app_adc_test.c
 *
 *  Created on: 2018Äê6ÔÂ28ÈÕ
 *      Author: jiachuang
 */

#include "app_adc.h"
#include "string.h"
#include "plf.h"
#include "clk_gate.h"// use 32K RC calibration result as seed
#include "LOG.h"
#include "rf_reg_settings.h"
#include "app_adc_utils.h"

//defines
#define ADC_CHANNEL_SUM_CNT     6       //all channel count
#define ADC_PER_CHANNEL_SAMPLE  100     //sample number per channel
#define ADC_TEST_LDO_FORCE_ON   1       //enable ldo force on
#define ADC_TEST_LDO_DELAY      8       //ldo delay number
#define ADC_TEST_GOAL           430     //the correct value of adc
#define ADC_TEST_TOL            50      //correct value tolerence


//macros
#define OUT_RANGE(val) ((val < (ADC_TEST_GOAL-ADC_TEST_TOL)) || (val > (ADC_TEST_GOAL+ADC_TEST_TOL)))

//variables
uint16_t adc_all_buff[ADC_CHANNEL_SUM_CNT][ADC_PER_CHANNEL_SAMPLE];
uint16_t error_rate[ADC_CHANNEL_SUM_CNT + 1];
uint32_t test_all_dma_sample_rate[3] = {1000,30000,100000};
uint32_t test_all_delay_rate[3]      = {900 ,30   ,2};

volatile uint16_t adc_test_all_dma_ok = 0;

app_adc_inst_t adc = ADC_INSTANCE(0);





static uint8_t adc_test_verify(void)
{
    uint32_t ch = 0;
    uint32_t times = 0;
    uint16_t err_sum=0;
    //calc errors
    memset(error_rate,0,sizeof(error_rate));
    for(times = 0 ; times <  ADC_PER_CHANNEL_SAMPLE; times ++)
    {
        for(ch = 0 ; ch <  ADC_CHANNEL_SUM_CNT; ch ++)
        {
            if(OUT_RANGE(adc_all_buff[ch][times]))
            {
                error_rate[ch]++;
            }
        }
    }
    //calc sum error
    for(ch = 0 ; ch < ADC_CHANNEL_SUM_CNT ; ch ++) 
    {
        err_sum += error_rate[ch];
    }
    error_rate[ADC_CHANNEL_SUM_CNT] = (err_sum + ADC_CHANNEL_SUM_CNT - 1) / ADC_CHANNEL_SUM_CNT;
    return err_sum;

}


static uint8_t adc_test_without_dma(app_adc_inst_t *inst , uint8_t sample_rate_index)
{
    uint32_t ch = 0;
    uint32_t times = 0;
    periph_err_t err;
    //init
    adc.param.ldo_force_on = ADC_TEST_LDO_FORCE_ON;
    adc.param.ldo_delay_us = ADC_TEST_LDO_DELAY;
    adc.param.use_dma = 0;
    app_adc_init(&inst->inst);
    //sample
    memset(adc_all_buff,0,sizeof(adc_all_buff));
    for(times = 0 ; times < ADC_PER_CHANNEL_SAMPLE ; times ++)
    {
        for(ch = 0 ; ch < ADC_CHANNEL_SUM_CNT ; ch ++)
        {
            err = app_adc_read_without_dma(&inst->inst , ch , &adc_all_buff[ch][times]);    //read adc
            if(err!=PERIPH_NO_ERROR) LOG(LOG_LVL_INFO , "adc read error!!\n");              //error handle
            BX_DELAY_US(test_all_delay_rate[sample_rate_index]);                            //delay for sample rate
        }
    }
    //uninit
    app_adc_uninit(&inst->inst);
    return adc_test_verify();
}



static void app_adc_rx_finish(void* ptr , uint8_t dummy)
{
	adc_test_all_dma_ok = 1;
}



static uint8_t adc_test_with_dma(app_adc_inst_t *inst , uint8_t sample_rate_index)
{
    uint32_t ch = 0;
    periph_err_t err;
    adc.param.ldo_force_on = ADC_TEST_LDO_FORCE_ON;
    adc.param.ldo_delay_us = ADC_TEST_LDO_DELAY;
    adc.param.use_dma = 1;
    //sample
    memset(adc_all_buff,0,sizeof(adc_all_buff));
    for(ch = 0 ; ch < ADC_CHANNEL_SUM_CNT ; ch ++)
    {
        BX_DELAY_US(100);
        adc_test_all_dma_ok = 0;
        adc.param.dma_bufptr = adc_all_buff[ch];
        adc.param.dma_size = ADC_PER_CHANNEL_SAMPLE;
        adc.param.dma_delay = APB_CLK / test_all_dma_sample_rate[sample_rate_index];
        //init
        app_adc_init(&inst->inst);
        err = app_adc_read_with_dma(&inst->inst , ch , app_adc_rx_finish , 0);
        if(err!=PERIPH_NO_ERROR) LOG(LOG_LVL_INFO , "adc read error!!\n");//error handle
        while(adc_test_all_dma_ok == 0);
        //uninit
        app_adc_uninit(&inst->inst);
    }    
    return adc_test_verify();
}




void app_adc_test(void)
{
    uint8_t sample_rate_idx=0;
    uint8_t err_num = 0;
    BX_DELAY_US(100*1000);//wait voltage stable
    while(*(__IO uint32_t *)4 != 0)
    {
        for(sample_rate_idx=0;sample_rate_idx<3;sample_rate_idx++)
        {
            //no dma
            err_num = adc_test_without_dma(&adc , sample_rate_idx);
            if(err_num > 0)LOG(LOG_LVL_INFO,"ADC NODMA ERROR! -- speed=%d\n",sample_rate_idx);
            else           LOG(LOG_LVL_INFO,"ADC NODMA OK!    -- speed=%d\n",sample_rate_idx);
            //use dma
            err_num = adc_test_with_dma(&adc , sample_rate_idx);
            if(err_num > 0)LOG(LOG_LVL_INFO,"ADC DMA   ERROR! -- speed=%d\n",sample_rate_idx);
            else           LOG(LOG_LVL_INFO,"ADC DMA   OK!    -- speed=%d\n",sample_rate_idx);
        }
    }

}

/////////////////////////////////////////////////////////////////////////////////////
#if ADC_GPADC_SINGLE_END_MODE_EN


#define ADC_DMA_TEST_SIZE   100
#define ADC_DMA_SAMPLE_RATE 1000
uint16_t adc_dma_buff[ADC_DMA_TEST_SIZE];
volatile uint8_t adc_dma_test_end = 0;

void adc_dma_test_cb(void* ptr , uint8_t dummy)
{
    adc_dma_test_end = 1;
}

void adc_dma_test(void)
{
    
    adc.param.ldo_force_on = ADC_TEST_LDO_FORCE_ON;
    adc.param.ldo_delay_us = ADC_TEST_LDO_DELAY;
    adc.param.use_dma = 1;
    adc.param.dma_bufptr = adc_dma_buff;
    adc.param.dma_size = ADC_DMA_TEST_SIZE;
    adc.param.dma_delay = APB_CLK / ADC_DMA_SAMPLE_RATE;
    
    //init
    adc_dma_test_end = 0;
    app_adc_init(&adc.inst);
    rf_setting_single_mode_adc();
    app_adc_read_with_dma(&adc.inst , 0 , adc_dma_test_cb , 0);
    while(adc_dma_test_end == 0);
    //uninit
    app_adc_uninit(&adc.inst);

    //calc value
    uint32_t sum1=0,sum2=0,voltage=0;
    for(uint32_t i=0;i<ADC_DMA_TEST_SIZE/2;i++)
    {
        sum1 += adc_dma_buff[i];
        sum2 += adc_dma_buff[i+ADC_DMA_TEST_SIZE/2];
    }
    sum1/=(ADC_DMA_TEST_SIZE/2);
    sum2/=(ADC_DMA_TEST_SIZE/2);
    voltage = adc_value_convert(sum1,sum2);
    LOG(LOG_LVL_INFO,"voltage=%dmV\n",voltage);
}

void adc_no_dma_test(void)
{
    uint32_t voltage = app_adc_gpadc_single_end(0);
    LOG(LOG_LVL_INFO,"voltage=%dmV\n",voltage);
}

#endif



