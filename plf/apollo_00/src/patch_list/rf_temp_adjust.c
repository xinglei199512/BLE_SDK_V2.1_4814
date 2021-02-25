#include "rf_temp_adjust.h"
#include "plf.h"
#include "log.h"
#include "rf_reg_typedef.h"
#include "FreeRTOS.h"
#include "timers.h"
//#include "app_adc_utils.h"
#include "sys_sleep.h"
#include "reg_ble_mdm.h"
#include "ll.h"
#include "rf_reg_settings.h"

/*
 *  schmitt TYPEDEF
 ****************************************************************************************
 */
#define SMT_TAB_SIZE    2
//goal 0 threshold
#define GOAL1_L   -5
#define GOAL1_H   5
//goal 1 threshold
#define GOAL2_L   45
#define GOAL2_H   55

//#define TEMP_TEST		MODE_OVER50//MODE_BELOW0/MODE_NORMAL/MODE_OVER50 車?車迆2谷?∩?D??米赤??REG2?那y那1車?㏒??y3㏒codeD豕辰a?芍㊣?

typedef int16_t smt_t;	//schmitt value
extern int32_t adc_cp_RO;
extern int16_t app_adc_tempSensor(void);

typedef enum
{
    SCHMITT_IN_LOW,
    SCHMITT_IN_HIGH,
}smt_stat_t;

typedef struct
{
    smt_t       goal_l;
    smt_t       goal_h;
    smt_stat_t  current;
}schmitt_t;

/*
 *  RF TYPEDEF
 ****************************************************************************************
 */

#define DISPLAY_PERIOD  1000

//temperature



#define DEFVAL           0xFF
 
#define portMAX_32_BIT_NUMBER       (0xffffffff)

#define REG4_10_9   hwp_rf_reg->rf_reg_4.VDD_AWO_Sleep 
#define REG9_22_20  hwp_rf_reg->rf_reg_9.RX_Mixer_LO_Bias_Voltage
#define REGA_6_4    hwp_rf_reg->rf_reg_a.LNA_I_Control_1
#define REGA_2_0    hwp_rf_reg->rf_reg_a.LNA_V_Contro
#define REGD_31_29  hwp_rf_reg->rf_reg_d.ICTL_PA1
#define REGD_28_26  hwp_rf_reg->rf_reg_d.ICTL_PA2
#define REGD_24_22  hwp_rf_reg->rf_reg_d.VCTL_0PA
#define REGE_5_3    hwp_rf_reg->rf_reg_e.VCO_Current_for_RX
#define REGE_11_9   hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_RX
#define REGE_2_0    hwp_rf_reg->rf_reg_e.VCO_Current_for_TX
#define REGE_8_6    hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_TX
#define REG0_29_28  hwp_rf_reg->rf_reg_0.xtal_current
#define REGB_15_14  hwp_rf_reg->rf_reg_b.VDD_PLL_Regulator_Voltage
#define REG6_6_3    hwp_rf_reg->rf_reg_6.Slow_Clock_Current
#define REG4_31_31  hwp_rf_reg->rf_reg_4.Sleep_Regulator

#define REGE_25_25  hwp_rf_reg->rf_reg_e.DM_Sync_LP
#define REGD_2_0    hwp_rf_reg->rf_reg_d.R1_TX_1Mbps
#define REGE_14_12  hwp_rf_reg->rf_reg_e.Divide_by_2_current_for_TX
#define REGB_13_12  hwp_rf_reg->rf_reg_b.VDD_Div2_Regulator_Voltage
#define REGB_11_10  hwp_rf_reg->rf_reg_b.VDD_VCO_Regulator_Voltage
#define REG0_31_30  hwp_rf_reg->rf_reg_0.iboost_current
#define REGF_2_0    hwp_rf_reg->rf_reg_f.R1_RX_1Mbps
#define REG4_25_22  hwp_rf_reg->rf_reg_4.VSLP
#define REG6_9      hwp_rf_reg->rf_reg_6.LV_32K
#define REG0_19_21  hwp_rf_reg->rf_reg_0.freq_pulling
 

 typedef struct
 {
    uint8_t reg4_10_9   [MODE_MAX];
    uint8_t reg9_22_20  [MODE_MAX];
    uint8_t rega_6_4    [MODE_MAX];
    uint8_t rega_2_0    [MODE_MAX];
    uint8_t regd_31_29  [MODE_MAX];
    uint8_t regd_28_26  [MODE_MAX];
    uint8_t regd_24_22  [MODE_MAX];
    uint8_t rege_5_3    [MODE_MAX];
    uint8_t rege_11_9   [MODE_MAX];
    uint8_t rege_2_0    [MODE_MAX];
    uint8_t rege_8_6    [MODE_MAX];
    uint8_t reg0_29_28  [MODE_MAX];
	uint8_t regb_15_14  [MODE_MAX];
	uint8_t reg6_6_3    [MODE_MAX];
	uint8_t reg4_31_31  [MODE_MAX];
    uint8_t rege_25_25  [MODE_MAX];
    uint8_t regd_2_0  	[MODE_MAX];
    uint8_t rege_14_12  [MODE_MAX];
    uint8_t regb_13_12  [MODE_MAX];
	uint8_t regb_11_10  [MODE_MAX];
    uint8_t reg0_31_30  [MODE_MAX];
    uint8_t regf_2_0    [MODE_MAX];
    uint8_t reg4_25_22  [MODE_MAX];
    uint8_t reg6_9      [MODE_MAX]; 
    uint8_t reg0_19_21  [MODE_MAX]; 
 }rf_adj_t;



void refresh_rf_regs_in_lp(void);
/*
 * VARIABLE
 ****************************************************************************************
 */

schmitt_t smt_tab[SMT_TAB_SIZE]=
{
    {GOAL1_L , GOAL1_H , SCHMITT_IN_LOW},
    {GOAL2_L , GOAL2_H , SCHMITT_IN_LOW},
};

rf_adj_t rf_adj;
int16_t current_temp=25;
//#define TEMP_TEST  MODE_NORMAL//MODE_OVER50//MODE_NORMAL//
#ifdef TEMP_TEST
temp_mode_t temp_mode=TEMP_TEST;
#else
temp_mode_t temp_mode=MODE_NORMAL;
#endif
StaticTimer_t xTimerDisplayBuffers_Temp;
TimerHandle_t xTimerDisplay;
uint16_t cur_deriv_calib[4];


#if (defined BX_TEMP_SENSOR) && (BX_TEMP_SENSOR == 1)  

/*
 * Schmitt tools
 ****************************************************************************************
 */

static uint8_t smt_get_range(smt_t val)
{
    smt_t current_goal;
    uint8_t result = 0;
    uint8_t i;
    //calc every channel output
    for(i=0;i<SMT_TAB_SIZE;i++)
    {
        current_goal = (smt_tab[i].current == SCHMITT_IN_LOW) ? smt_tab[i].goal_l : smt_tab[i].goal_h;
        if(val > current_goal)
        {
            smt_tab[i].current = SCHMITT_IN_LOW;
            result++;
        }
        else
        {
            smt_tab[i].current = SCHMITT_IN_HIGH;
        }
    }
    return result;
}



/*
 * DEBUG tools
 ****************************************************************************************
 */
#ifdef ENABLE_RF_ADJ_LOG
static void print_full_reg(TimerHandle_t xTimer)
{
    LOG(3,"\r\n");
    LOG(3,"0=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_0));
    LOG(3,"1=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_1));
    LOG(3,"2=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_2));
    LOG(3,"3=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_3));
    LOG(3,"4=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_4));
    LOG(3,"5=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_5));
    LOG(3,"6=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_6));
    LOG(3,"7=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_7));
    LOG(3,"8=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_8));
    LOG(3,"9=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_9));
    LOG(3,"a=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_a));
    LOG(3,"b=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_b));
    LOG(3,"c=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_c));
    LOG(3,"d=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_d));
    LOG(3,"e=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_e));
    LOG(3,"f=%8x\r\n",*((uint32_t*)&hwp_rf_reg->rf_reg_f));
    BX_DELAY_US(100*11*16);

    LOG(3,"\tT=%d,temp_mode=%d\n",current_temp,temp_mode);
	if(temp_mode == 0)
	{
		LOG(3,"xtal startup time=%d\r\n",XTAL_STARTUP_TIME_50);
	}
	else
	{
		LOG(3,"xtal startup time=%d\r\n",XTAL_STARTUP_TIME);
	}

//	LOG(3,"ble mdm vcocali capdev 2=%8x\r\n",ble_mdm_vcocali_capdev_2_getf());
//	LOG(3,"ble mdm vcocali capdev 3=%8x\r\n",ble_mdm_vcocali_capdev_3_getf());
//	LOG(3,"ble mdm vcocali capdev 4=%8x\r\n",ble_mdm_vcocali_capdev_4_getf());

	LOG(3,"ble mdm vcocali capdev 2=%8x\r\n",cur_deriv_calib[1]);
	LOG(3,"ble mdm vcocali capdev 3=%8x\r\n",cur_deriv_calib[2]);
	LOG(3,"ble mdm vcocali capdev 4=%8x\r\n",cur_deriv_calib[3]);
    BX_DELAY_US(100*30);
}
#endif
/*
 * RF tools
 ****************************************************************************************
 */

static temp_mode_t mode_last;

void init_rf_temp_adjust(void)
{   
    uint8_t* b;
    b = rf_adj.reg4_10_9 ; b[0]=b[1]=b[2] = REG4_10_9 ;   b[MODE_BELOW0] =  0; //VDD_AWO_Sleep
    b = rf_adj.reg9_22_20; b[0]=b[1]=b[2] = REG9_22_20;   b[MODE_OVER50] -= 1; //RX_Mixer_LO_Bias_Voltage
    b = rf_adj.rega_6_4  ; b[0]=b[1]=b[2] = REGA_6_4  ;   b[MODE_OVER50] =  6; //LNA_I_Control_1
    b = rf_adj.rega_2_0  ; b[0]=b[1]=b[2] = REGA_2_0  ;   b[MODE_OVER50] =  6; //LNA_V_Contro
    b = rf_adj.regd_31_29; b[0]=b[1]=b[2] = REGD_31_29;   b[MODE_OVER50] += 1; //ICTL_PA1
    b = rf_adj.regd_28_26; b[0]=b[1]=b[2] = REGD_28_26;   b[MODE_OVER50] =  7; //ICTL_PA2
    b = rf_adj.regd_24_22; b[0]=b[1]=b[2] = REGD_24_22;   b[MODE_OVER50] -= 1; //VCTL_0PA

    b = rf_adj.rege_5_3  ; b[0]=b[1]=b[2] = REGE_5_3  ;   b[MODE_BELOW0] = 2;  // VCO_Current_for_RX
    b = rf_adj.rege_11_9 ; b[0]=b[1]=b[2] = REGE_11_9 ;   b[MODE_BELOW0] += 1; //VCO_Buffer_Current_for_RX
    b = rf_adj.rege_2_0  ; b[0]=b[1]=b[2] = REGE_2_0  ;   b[MODE_BELOW0] += 1;//b[MODE_BELOW0] += 3; VCO_Current_for_TX
    b = rf_adj.rege_8_6  ; b[0]=b[1]=b[2] = REGE_8_6  ;   b[MODE_BELOW0] += 1; //VCO_Buffer_Current_for_TX
	b = rf_adj.regb_15_14; b[0]=b[1]=b[2] = REGB_15_14;   b[MODE_BELOW0] = 1; //VDD_PLL_Regulator_Voltage
	b = rf_adj.reg6_6_3;   b[0]=b[1]=b[2] = REG6_6_3;     b[MODE_BELOW0] = 0x2; //Slow_Clock_Current
	//b = rf_adj.reg6_6_3;   b[0]=b[1]=b[2] = REG6_6_3;     b[MODE_BELOW0] = 0x0; 
	//b = rf_adj.reg0_29_28; b[0]=b[1]=b[2] = REG0_29_28;   b[MODE_BELOW0] = 0; //xtal_current 20200423 org:2
	b = rf_adj.reg4_31_31; b[0]=b[1]=b[2] = REG4_31_31;   b[MODE_BELOW0] = 0; //Sleep_Regulator
	b = rf_adj.rege_25_25; b[0]=b[1]=b[2] = REGE_25_25;   b[MODE_BELOW0] = 1; //DM_Sync_LP jian 20200522 org:0
	b = rf_adj.regd_2_0;   b[0]=b[1]=b[2] = REGD_2_0;     b[MODE_BELOW0] = 0; //R1_TX_1Mbps
	b = rf_adj.rege_14_12; b[0]=b[1]=b[2] = REGE_14_12;   b[MODE_BELOW0] = 7; //Divide_by_2_current_for_TX
	b = rf_adj.regb_13_12; b[0]=b[1]=b[2] = REGB_13_12;   b[MODE_BELOW0] = 1; //VDD_Div2_Regulator_Voltage
    b = rf_adj.reg0_31_30; b[0]=b[1]=b[2] = REG0_31_30;   b[MODE_BELOW0] = 0; //iboost_current
	b = rf_adj.regb_11_10; b[0]=b[1]=b[2] = REGB_11_10;   b[MODE_BELOW0] = 1; //VDD_VCO_Regulator_Voltage
	b = rf_adj.regf_2_0;   b[0]=b[1]=b[2] = REGF_2_0;     b[MODE_BELOW0] = 1;	//	R1_RX_1Mbps
	b = rf_adj.reg4_25_22; b[0]=b[1]=b[2] = REG4_25_22;   b[MODE_BELOW0] = 15; //VSLP
	b = rf_adj.reg6_9;     b[0]=b[1]=b[2] = REG6_9;       b[MODE_BELOW0] = 0; //LV_32K
	b = rf_adj.reg0_19_21; b[0]=b[1]=b[2] = REG0_19_21;   b[MODE_OVER50] = 0; //freq_pulling

    //print
    current_temp = app_adc_tempSensor();
    
//    extern uint32_t config_and_enable_32m_xtal();
//	uint32_t current_time = config_and_enable_32m_xtal();
	#ifdef TEMP_TEST
	temp_mode = mode_last = TEMP_TEST;
	#else
	temp_mode = mode_last = (temp_mode_t)smt_get_range(current_temp);
	#endif
	if(temp_mode == 0)
	{
			reg_set_xtal_current_below_temp0();
	}
	else
	{
			reg_set_xtal_current_normal();
	}
//	reg_set_xtal_current_normal();
	
	refresh_rf_regs_in_lp();
/*	if(mode == 0)
	{
		switch_to_32m_xtal(current_time, mode);
	}
	else
	{
		switch_to_32m_xtal(current_time, mode);
	}
*/	

#ifdef ENABLE_RF_ADJ_LOG
    LOG(3,"mode0:<0,mode1:0-50,mode2:>50\r\n");
    LOG(3,"RO=%d\r\n===INIT===\r\n",adc_cp_RO);
    BX_DELAY_US(100*100);
    print_full_reg( NULL );
    //timer tprint
    xTimerDisplay = xTimerCreateStatic( "xTimerDisplay", pdMS_TO_TICKS(DISPLAY_PERIOD), pdTRUE, ( void * )0, print_full_reg, &xTimerDisplayBuffers_Temp);
    xTimerStart(xTimerDisplay, pdFALSE);
#endif
}

void try_to_update_rf_param_with_temp(void)
{
    static uint32_t tick_count_last = 0;
    uint32_t tick_count_current = xTaskGetTickCount();
    uint32_t tick_temp;
    if(tick_count_current > tick_count_last)
    {
        tick_temp = tick_count_current - tick_count_last;
    }
    else if(tick_count_current < tick_count_last)
    {   
        tick_temp = (portMAX_32_BIT_NUMBER - tick_count_last) + tick_count_current;
    }
    
//    LOG_I("tick_temp = %d",tick_temp);
    if( tick_temp >= configTICK_RATE_HZ )
    {
//        LOG_I("handle_read_temp");
        tick_count_last = tick_count_current;
        handle_read_temp();//try to switch RF PARAM
    }

}



void handle_read_temp(void)
{
    current_temp = app_adc_tempSensor();
	#ifdef TEMP_TEST
	  temp_mode = TEMP_TEST;
	#else
	  temp_mode = (temp_mode_t)smt_get_range(current_temp);
	#endif
		//if(mac_status != sleep_low_power_clk){return;}
    if( temp_mode != mode_last )
    {
        mode_last = temp_mode;
        refresh_rf_regs_in_lp();
    }
}

#if 0
//because post_deepsleep_processing_mp()->config_and_enable_32m_xtal() and switch_to_32m_xtal() will change the reg0.
//we should revert this reg after that function's change.
static void refresh_reg0_after_sleep(void)
{
    //REG0_29_28 = rf_adj.reg0_29_28 [temp_mode];
}
#endif

void refresh_rf_regs_in_lp(void)
{
	LOG(3,"refresh_rf_regs_in_lp\n");
	if(temp_mode == MODE_BELOW0)
	{
		ble_mdm_vcocali_capdev_2_setf(0x810);	
		ble_mdm_vcocali_capdev_3_setf(0x810);
		ble_mdm_vcocali_capdev_4_setf(0x78F);
	}
	else
	{
		ble_mdm_vcocali_capdev_2_setf(0x78F);
		ble_mdm_vcocali_capdev_3_setf(0x78F);
		ble_mdm_vcocali_capdev_4_setf(0x78E);
	}

	cur_deriv_calib[0] = ble_mdm_vcocali_capdev_1_getf();
	cur_deriv_calib[1] = ble_mdm_vcocali_capdev_2_getf();
	cur_deriv_calib[2] = ble_mdm_vcocali_capdev_3_getf();
	cur_deriv_calib[3] = ble_mdm_vcocali_capdev_4_getf();

    REG4_10_9  = rf_adj.reg4_10_9  [temp_mode];
    REG9_22_20 = rf_adj.reg9_22_20 [temp_mode];
    REGA_6_4   = rf_adj.rega_6_4   [temp_mode];
    REGA_2_0   = rf_adj.rega_2_0   [temp_mode];
    REGD_31_29 = rf_adj.regd_31_29 [temp_mode];
    REGD_28_26 = rf_adj.regd_28_26 [temp_mode];
    REGD_24_22 = rf_adj.regd_24_22 [temp_mode];
    REGE_5_3   = rf_adj.rege_5_3   [temp_mode];
    REGE_11_9  = rf_adj.rege_11_9  [temp_mode];
    REGE_2_0   = rf_adj.rege_2_0   [temp_mode];
    REGE_8_6   = rf_adj.rege_8_6   [temp_mode];
    //REG0_29_28 = rf_adj.reg0_29_28 [temp_mode];
	REGB_15_14 = rf_adj.regb_15_14 [temp_mode];
	REG6_6_3 = rf_adj.reg6_6_3 [temp_mode];
	REG4_31_31 = rf_adj.reg4_31_31[temp_mode];
	REGE_25_25 = rf_adj.rege_25_25[temp_mode];
	REGD_2_0 = rf_adj.regd_2_0[temp_mode];
	REGE_14_12 = rf_adj.rege_14_12[temp_mode];
	REGB_13_12 = rf_adj.regb_13_12[temp_mode];
	REGB_11_10 = rf_adj.regb_11_10[temp_mode];
    REG0_31_30 = rf_adj.reg0_31_30[temp_mode];
    //REG0_29_28 = rf_adj.reg0_29_28[temp_mode];   //200522
    REGF_2_0   = rf_adj.regf_2_0[temp_mode];
    REG4_25_22 = rf_adj.reg4_25_22[temp_mode];
    REG6_9 = rf_adj.reg6_9[temp_mode];
    REG0_19_21 = rf_adj.reg0_19_21[temp_mode];
	
}

#endif
