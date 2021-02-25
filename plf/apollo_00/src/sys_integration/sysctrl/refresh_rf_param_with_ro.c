/*
 * refresh_rf_param_with_ro.c
 *
 *  Created on: 2019�~7��10��
 *      Author: user
 */
#include "bx_config.h"
#include "modem.h"
#include "rf_reg_typedef.h"

#if RF_PARAM == 2
void refresh_rf_param_with_ro(uint32_t ro)
{
//    if((ro >= 0) && (ro <= 31)) //befroe bin3
//    {
//        //set vco param
//        set_vco_buff_others(0x78f,0x78e,0x78e);
//        set_vco_buff_aa55(0x811,0x810,0x810);
//        set_vco_buff_0_aa55(0x123);
//        set_vco_buff_0_others(0x456);
//        //set rf reg
//        hwp_rf_reg->rf_reg_b.VDD_VCO_Regulator_Voltage  = 0;
//        hwp_rf_reg->rf_reg_b.VDD_PLL_Regulator_Voltage  = 1;
//        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 2;
//        hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_TX  = 1;
//        hwp_rf_reg->rf_reg_e.Divide_by_2_current_for_TX = 7;
//    }
    if((ro >=0) && (ro <= 28)) //binG12
    {
        //set vco param
        set_vco_buff_others(0x78f,0x78e,0x78e);
        set_vco_buff_aa55(0x811,0x810,0x810);
//        set_vco_buff_0_aa55(0x811);
//        set_vco_buff_0_others(0x78f);
        //set rf reg
        hwp_rf_reg->rf_reg_b.VDD_VCO_Regulator_Voltage  = 0;
        hwp_rf_reg->rf_reg_b.VDD_PLL_Regulator_Voltage  = 1;
        hwp_rf_reg->rf_reg_b.VDD_Div2_Regulator_Voltage = 1;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 2;
        hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_TX  = 2;
        hwp_rf_reg->rf_reg_e.Divide_by_2_current_for_TX = 6;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_RX            = 2;
        hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_RX  = 2;
    }
    if((ro >=29) && (ro <= 40)) //bin34AB
    {
        //set vco param
        set_vco_buff_others(0x78f,0x78e,0x78e);
        set_vco_buff_aa55(0x811,0x810,0x810);
//        set_vco_buff_0_aa55(0x811);
//        set_vco_buff_0_others(0x78f);
        //set rf reg
        hwp_rf_reg->rf_reg_b.VDD_VCO_Regulator_Voltage  = 0;
        hwp_rf_reg->rf_reg_b.VDD_PLL_Regulator_Voltage  = 1;
        hwp_rf_reg->rf_reg_b.VDD_Div2_Regulator_Voltage = 1;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 3;
        hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_TX  = 1;
        hwp_rf_reg->rf_reg_e.Divide_by_2_current_for_TX = 6;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_RX            = 2;
        hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_RX  = 2;
    }
    if((ro >= 41) && (ro <= 52)) //binCDEF
    {
        //set vco param
        set_vco_buff_others(0x78f,0x78e,0x78e);
        set_vco_buff_aa55(0x811,0x810,0x810);
        //set rf reg
        hwp_rf_reg->rf_reg_b.VDD_VCO_Regulator_Voltage  = 0;
        hwp_rf_reg->rf_reg_b.VDD_PLL_Regulator_Voltage  = 1;
        hwp_rf_reg->rf_reg_b.VDD_Div2_Regulator_Voltage = 2;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 3;
        hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_TX  = 5;
        hwp_rf_reg->rf_reg_e.Divide_by_2_current_for_TX = 7;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_RX            = 3;
        hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_RX  = 2;
    }
//    if((ro >= 50) && (ro <= 52)) //binF
//    {
//        //set vco param
//        set_vco_buff_others(0x78f,0x78e,0x78e);
//        set_vco_buff_aa55(0x811,0x810,0x810);
//        //set rf reg
//        hwp_rf_reg->rf_reg_b.VDD_VCO_Regulator_Voltage  = 0;
//        hwp_rf_reg->rf_reg_b.VDD_PLL_Regulator_Voltage  = 1;
//        hwp_rf_reg->rf_reg_b.VDD_Div2_Regulator_Voltage = 2;
//        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 3;
//        hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_TX  = 5;
//        hwp_rf_reg->rf_reg_e.Divide_by_2_current_for_TX = 7;
//        hwp_rf_reg->rf_reg_e.VCO_Current_for_RX            = 3;
//        hwp_rf_reg->rf_reg_e.VCO_Buffer_Current_for_RX  = 2;
//    }
    enable_vco_value(false);
}
#elif RF_PARAM == 3
void refresh_rf_param_with_ro(uint32_t ro)
{
    if((ro > 0) && (ro <= 28)) //binG12
    {
        //set_vco_buff_others(0x78f,0x78e,0x78e);
        //set_vco_buff_aa55(0x811,0x810,0x810);
        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 3;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_RX            = 2;

        if(MAIN_CLOCK > 32000000)
        {
            hwp_rf_reg->rf_reg_b.VDD_PLL_Regulator_Voltage = 0;
        }
        
    }
	
    if((ro >=29) && (ro <= 40)) //bin34AB
    {
        //set_vco_buff_others(0x78f,0x78e,0x78e);
        //set_vco_buff_aa55(0x811,0x810,0x810);    
        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 4;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_RX            = 2;

        if(MAIN_CLOCK > 32000000)
        {
            hwp_rf_reg->rf_reg_b.VDD_PLL_Regulator_Voltage = 0;
        }
    }
    if((ro >= 41) && (ro <= 46)) 
    {
        //set_vco_buff_others(0x810,0x78f,0x78f);
        //set_vco_buff_aa55(0x812,0x811,0x811);    
        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 4;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_RX            = 3;
    }
    if((ro >= 47) && (ro <= 52)) 
    {
        //set_vco_buff_others(0x810,0x78f,0x78f);
        //set_vco_buff_aa55(0x812,0x811,0x811);    
        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 5;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_RX            = 4;
    }
    if(ro >= 53) 
    {
        //set_vco_buff_others(0x810,0x78f,0x78f);
        //set_vco_buff_aa55(0x812,0x811,0x811);
        hwp_rf_reg->rf_reg_e.VCO_Current_for_TX         = 5;
        hwp_rf_reg->rf_reg_e.VCO_Current_for_RX            = 4;
    }
    //set vco param
    enable_vco_value(false);
}

#endif
