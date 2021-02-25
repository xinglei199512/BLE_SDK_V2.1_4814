/**
 ****************************************************************************************
 *
 * @file   aes_128_test.c
 *
 * @brief  .
 *
 * @author  jiachuang
 * @date    2018-12-10 11:24
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
#include "aes_128.h"
#ifdef UNIT_TEST_SECURITY_AES_128






/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
//----- TEST PASS
#define K_LEN   16 //key
static aes_128_buf_t aes_128_buf;
uint8_t test_key[K_LEN]   = {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
uint8_t test_src[K_LEN]   = {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f};
//correct encryped value = 80 D2 A5 B0 8F A0 EE 51 14 3B 45 9E 63 81 06 DF
uint8_t test_enc[K_LEN]   = {0};
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */


void aes_128_test_callback(struct ring_array_async_process_base_s *aaa, void *bbb,uint8_t ccc)
{
    LOG(LOG_LVL_INFO , "aes_128_test_callback\n");
}

void new_impl_test(void)
{
    LOG(LOG_LVL_INFO , "new_impl_test\n");
    aes_128_buf.async_base.callback =aes_128_test_callback;
    aes_128_buf.param.encrypted = test_enc;
    aes_128_buf.param.key = test_key;
    aes_128_buf.param.plain = test_src;

    aes_128_start(&aes_128_buf);
    LOG(LOG_LVL_INFO , "aes_128_start\n");
}


#endif
