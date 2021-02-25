//////////////////////////////////////////////////////////////////////////////
//  Copyright (C) by BLUEX.
//  This module is a confidential and proprietary property of BLUEX
//  and a possession or use of this module requires written permission 
//  from BLUEX.
//----------------------------------------------------------------------------
// Author           : None
// Company          : BLUEX
// ---------------------------------------------------------------------------
// Description      :
// Simulation Notes :
// Synthesis Notes  :
// Application Note :
// Simulator        :
// Parameters       :
// Terms & concepts :
// Bugs             :
// Open issues and future enhancements :
// References       :
// Revision History :
// ---------------------------------------------------------------------------
//
//////////////////////////////////////////////////////////////////////////////
#ifndef __REG_WDT_H__
#define __REG_WDT_H__
//Auto-gen by fr
#include "_reg_base_addr.h"
#include <stdint.h>


typedef struct
{
    volatile uint32_t WDT_CR;
    volatile uint32_t WDT_TORR;
    volatile uint32_t WDT_CCVR;
    volatile uint32_t WDT_CRR;
    volatile uint32_t WDT_STAT;
    volatile uint32_t WDT_EOI;
    volatile uint32_t reserver[51];
    volatile uint32_t WDT_COMP_PARAMS_5;
    volatile uint32_t WDT_COMP_PARAMS_4;
    volatile uint32_t WDT_COMP_PARAMS_3;
    volatile uint32_t WDT_COMP_PARAMS_2;
    volatile uint32_t WDT_COMP_PARAMS_1;
    volatile uint32_t WDT_COMP_VERSION;
    volatile uint32_t WDT_COMP_TYPE;
}reg_wdt_t;

enum WDT_WDT_CR_FIELD
{
    WDT_WDT_CR_MASK = 0xffffff,
    WDT_WDT_CR_POS  = 0,
    WDT_RPL_MASK    = 0x1C,
    WDT_RPL_POS     = 2,
    WDT_RMOD_MASK   = 0x02,
    WDT_RMOD_POS    = 1,
    WDT_VAL_MASK    = 0x01,
    WDT_VAL_POS     = 0,
};

enum WDT_WDT_TORR_FIELD
{
    WDT_WDT_TORR_MASK = 0xffffff,
    WDT_WDT_TORR_POS = 0,
    WDT_TOP_MASK     = 0x0f,
    WDT_TOP_POS      = 0,
};

enum WDT_WDT_CCVR_FIELD
{
    WDT_WDT_CCVR_MASK = 0xffffff,
    WDT_WDT_CCVR_POS = 0,
};

enum WDT_WDT_CRR_FIELD
{
    WDT_WDT_CRR_MASK = 0xffffff,
    WDT_WDT_CRR_POS = 0,
    WDT_COUNT_RESTART_REGISTER_MASK = 0xff,
    WDT_COUNT_RESTART_REGISTER_POS = 0,
};

enum WDT_WDT_STAT_FIELD
{
    WDT_WDT_STAT_MASK = 0xffffff,
    WDT_WDT_STAT_POS = 0,
};

enum WDT_WDT_EOI_FIELD
{
    WDT_WDT_EOI_MASK = 0xffffff,
    WDT_WDT_EOI_POS = 0,
    WDT_INTERRUPTCLEARREGISTER_MASK = 0x01,
    WDT_INTERRUPTCLEARREGISTER_POS  = 0,
};

enum WDT_WDT_COMP_PARAMS_5_FIELD
{
    WDT_WDT_COMP_PARAMS_5_MASK = 0xffffff,
    WDT_WDT_COMP_PARAMS_5_POS = 0,
};

enum WDT_WDT_COMP_PARAMS_4_FIELD
{
    WDT_WDT_COMP_PARAMS_4_MASK = 0xffffff,
    WDT_WDT_COMP_PARAMS_4_POS = 0,
};

enum WDT_WDT_COMP_PARAMS_3_FIELD
{
    WDT_WDT_COMP_PARAMS_3_MASK = 0xffffff,
    WDT_WDT_COMP_PARAMS_3_POS = 0,
};

enum WDT_WDT_COMP_PARAMS_2_FIELD
{
    WDT_WDT_COMP_PARAMS_2_MASK = 0xffffff,
    WDT_WDT_COMP_PARAMS_2_POS = 0,
};

enum WDT_WDT_COMP_PARAMS_1_FIELD
{
    WDT_WDT_COMP_PARAMS_1_MASK = 0xffffff,
    WDT_WDT_COMP_PARAMS_1_POS = 0,
};

enum WDT_WDT_COMP_VERSION_FIELD
{
    WDT_WDT_COMP_VERSION_MASK = 0xffffff,
    WDT_WDT_COMP_VERSION_POS = 0,
};

enum WDT_WDT_COMP_TYPE_FIELD
{
    WDT_WDT_COMP_TYPE_MASK = 0xffffff,
    WDT_WDT_COMP_TYPE_POS = 0,
};

#endif

