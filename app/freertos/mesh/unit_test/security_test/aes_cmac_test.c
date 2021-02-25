/**
 ****************************************************************************************
 *
 * @file   aes_cmac_test.c
 *
 * @brief  .
 *
 * @author  jiachuang
 * @date    2018-12-10 17:40
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) BlueX Microelectronics 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"
#include "unit_test_config.h"
#include "log.h"
#include "aes_ccm_cmac.h"

#ifdef UNIT_TEST_SECURITY_AES_CMAC_BASIC

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
#define AES_CMAC_EXAMPLE2 0
#define AES_CMAC_EXAMPLE4 1


#if AES_CMAC_EXAMPLE2        // AES CMAC Example 2   ---- TEST PASS
    #define CMAC_KEY_LEN    16
    #define CMAC_M_LEN      16
    #define CMAC_RES_LEN    16

    uint8_t cmac_key[CMAC_KEY_LEN] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
    uint8_t cmac_msg[CMAC_M_LEN  ] = {0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A};
    uint8_t cmac_res[CMAC_RES_LEN] = {0};
    /* correct answer:
    Tag is
    070A16B4 6B4D4144 F79BDD9D D04A287C
     */
#endif

#if AES_CMAC_EXAMPLE4       // AES CMAC Example 4   ---- TEST PASS
    #define CMAC_KEY_LEN    16
    #define CMAC_M_LEN      64
    #define CMAC_RES_LEN    16

    uint8_t cmac_key[CMAC_KEY_LEN] = {0x2B,0x7E,0x15,0x16,0x28,0xAE,0xD2,0xA6,0xAB,0xF7,0x15,0x88,0x09,0xCF,0x4F,0x3C};
    uint8_t cmac_msg[CMAC_M_LEN  ] = {
    0x6B,0xC1,0xBE,0xE2,0x2E,0x40,0x9F,0x96,0xE9,0x3D,0x7E,0x11,0x73,0x93,0x17,0x2A,
    0xAE,0x2D,0x8A,0x57,0x1E,0x03,0xAC,0x9C,0x9E,0xB7,0x6F,0xAC,0x45,0xAF,0x8E,0x51,
    0x30,0xC8,0x1C,0x46,0xA3,0x5C,0xE4,0x11,0xE5,0xFB,0xC1,0x19,0x1A,0x0A,0x52,0xEF,
    0xF6,0x9F,0x24,0x45,0xDF,0x4F,0x9B,0x17,0xAD,0x2B,0x41,0x7B,0xE6,0x6C,0x37,0x10,
    };
    uint8_t cmac_res[CMAC_RES_LEN] = {0};
    /* correct answer:
     Tag is
    51F0BEBF 7E3B9D92 FC497417 79363CFE
     */
#endif


ccm_cmac_buf_t ccm_cmac_buf;

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */


void aes_ccm_cmac_test_callback(struct ring_array_async_process_base_s *aaa, void *bbb,uint8_t ccc)
{
    LOG(LOG_LVL_INFO , "aes_ccm_cmac_test_callback\n");
}

void new_impl_test(void)
{
    LOG(LOG_LVL_INFO , "new_impl_test\n");
    ccm_cmac_buf.async_base.callback = aes_ccm_cmac_test_callback;
    ccm_cmac_buf.op_type = CMAC_CALC;

    ccm_cmac_buf.param.cmac.rslt = cmac_res;
    ccm_cmac_buf.param.cmac.k = cmac_key;
    ccm_cmac_buf.param.cmac.length = CMAC_M_LEN;
    ccm_cmac_buf.param.cmac.m = cmac_msg;

    ccm_cmac_start(&ccm_cmac_buf);
    LOG(LOG_LVL_INFO , "ccm_cmac_start\n");
}


#endif



