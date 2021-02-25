/**
 ****************************************************************************************
 *
 * @file   aes_ccm_test.c
 *
 * @brief  .
 *
 * @author  jiachuang
 * @date    2018-12-10 16:21
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


#ifdef UNIT_TEST_SECURITY_AES_CCM_ENCRYPT

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
#if 1       //AES_CCM Example 4   ---- TEST PASS
    //WARNING: You should change macro : NONCE_LENGTH to 7
    #define K_LEN   16 //key
    #define T_LEN   4  //mic length
    #define N_LEN   7  //nonce
    #define A_LEN   0  //additional data
    #define P_LEN   64  //msg payload

    uint8_t test_key[K_LEN]   = {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
    uint8_t test_nonce[N_LEN] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16};
    #define  test_addi NULL ;
    uint8_t test_msg[P_LEN]   = {
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f
    };
    /*  correct encrypted_data:
    C is
    7162015B C051951E 5918AEAF 3C11F3D4
    AC363F8D 5B6AF3D3 69603B04 F24CAE29
    964E2F2B F9D31143 F72527CE 2DB402EA
    B7660E4A 10B08E82 266517CD F60267F9
    C66B655C
     */
#endif


uint8_t test_mesh_encrypted_data[T_LEN+P_LEN]={0};
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
    ccm_cmac_buf.op_type = CCM_ENCRYPT;
    ccm_cmac_buf.param.ccm.rslt = test_mesh_encrypted_data;
    ccm_cmac_buf.param.ccm.key               = test_key;
    ccm_cmac_buf.param.ccm.nonce             = test_nonce;
    ccm_cmac_buf.param.ccm.msg               = test_msg;
    ccm_cmac_buf.param.ccm.additional_data   = test_addi;
    ccm_cmac_buf.param.ccm.msg_length        = P_LEN;
    ccm_cmac_buf.param.ccm.mic_length        = T_LEN;
    ccm_cmac_buf.param.ccm.additional_data_length = A_LEN;

    ccm_cmac_start(&ccm_cmac_buf);
    LOG(LOG_LVL_INFO , "ccm_cmac_start\n");
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef UNIT_TEST_SECURITY_AES_CCM_DECRYPT

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
#if 1       //AES_CCM Example 2   ---- TEST PASS
    //WARNING: You should change macro : NONCE_LENGTH to 8
/*    #define K_LEN   16 //key
    #define T_LEN   6  //mic length
    #define N_LEN   8  //nonce=8
    #define A_LEN   16  //additional data
    #define P_LEN   16  //msg payload

    uint8_t test_key[K_LEN]   = {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
    uint8_t test_nonce[N_LEN] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
    uint8_t test_addi[A_LEN]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    uint8_t test_msg[T_LEN+P_LEN]   = {0};
    uint8_t test_c[T_LEN+P_LEN]     ={0xd2,0xa1,0xf0,0xe0,0x51,0xea,0x5f,0x62,0x08,0x1a,0x77,0x92,0x07,0x3d,0x59,0x3d,0x1f,0xc6,0x4f,0xbf,0xac,0xcd};
    */
    //  correct source data :
   // P is
   // 20212223 24252627 28292A2B 2C2D2E2F

    #define K_LEN   32 //key
    #define T_LEN   4  //mic length
    #define N_LEN   7  //nonce=8
    #define A_LEN   0  //additional data
    #define P_LEN   64  //msg payload

    uint8_t test_key[K_LEN]   = {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d,0x5e,0x5f};
    uint8_t test_nonce[N_LEN] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16};
    //uint8_t test_addi[A_LEN]  = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
    uint8_t test_msg[T_LEN+P_LEN]   = {0};
    uint8_t test_c[T_LEN+P_LEN]     =
{   0x71,0x62,0x01,0x5B ,0xC0,0x51,0x95,0x1E ,0x59,0x18,0xAE,0xAF ,0x3C,0x11,0xF3,0xD4
,0xAC,0x36,0x3F,0x8D ,0x5B,0x6A,0xF3,0xD3,0x69,0x60,0x3B,0x04,0xF2,0x4C,0xAE,0x29
,0x96,0x4E,0x2F,0x2B,0xF9,0xD3,0x11,0x43,0xF7,0x25,0x27,0xCE ,0x2D,0xB4,0x02,0xEA
,0xB7,0x66,0x0E,0x4A ,0x10,0xB0,0x8E,0x82 ,0x26,0x65,0x17,0xCD ,0xF6,0x02,0x67,0xF9
,0xC6,0x6B,0x65,0x5C};


    /*
    correct source data :
    P is

    20212223 24252627 28292A2B 2C2D2E2F
    30313233 34353637 38393A3B 3C3D3E3F
    40414243 44454647 48494A4B 4C4D4E4F
    50515253 54555657 58595A5B 5C5D5E5F
    */
     
#endif


ccm_cmac_buf_t ccm_cmac_buf;

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void aes_ccm_cmac_test_callback(ccm_cmac_buf_t    *aaa, void *bbb,uint8_t ccc)
{
    LOG(LOG_LVL_INFO , "aes_ccm_cmac_test_callback\n");
}

void new_impl_test(void)
{
    LOG(LOG_LVL_INFO , "new_impl_test\n");
    ccm_cmac_buf.op_type = CCM_DECRYPT;
    ccm_cmac_buf.param.ccm.rslt = test_msg;
    //init
    ccm_cmac_buf.param.ccm.key               = test_key;
    ccm_cmac_buf.param.ccm.nonce             = test_nonce;
    ccm_cmac_buf.param.ccm.msg               = test_c;
  //  ccm_cmac_buf.param.ccm.additional_data   = test_addi;
    ccm_cmac_buf.param.ccm.msg_length        = P_LEN;
    ccm_cmac_buf.param.ccm.mic_length        = T_LEN;
    ccm_cmac_buf.param.ccm.additional_data_length = A_LEN;

    ccm_cmac_start(&ccm_cmac_buf,aes_ccm_cmac_test_callback);
    LOG(LOG_LVL_INFO , "ccm_cmac_start\n");
}

#endif

