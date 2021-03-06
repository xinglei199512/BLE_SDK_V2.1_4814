#include "aes_ecb_blocked.h"
#include "log.h"
#include "osapp_utils.h"


//encrypt
uint8_t enc_key   [16]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02};
uint8_t enc_input [16]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x55,0x55,0x55,0x55,0x44,0x44,0x44,0x44};
uint8_t enc_output[16];//answer=01 66 71 53 0E F7 F3 A6 E6 54 92 AD 33 ED 0A 70 
//decrypt
uint8_t dec_key   [16]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x22,0x22,0x22,0x22,0x55,0x55,0x55,0x55};
uint8_t dec_input [16]={0xB8,0xCF,0x38,0xE9,0x19,0xE5,0xDD,0x15,0x4A,0x6E,0x40,0x31,0x16,0x4E,0x05,0xA5};
uint8_t dec_output[16];//answer=01 02 03 04 05 06 07 08 55 55 55 55 11 11 11 11 


void aes_block_test(void)
{
    aes_ecb_bloked_operation_rev(enc_key , enc_input , enc_output , AES_ECB_BLOCKED_ENCRYPT);
    aes_ecb_bloked_operation_rev(dec_key , dec_input , dec_output , AES_ECB_BLOCKED_DECRYPT);
    
    LOG(LOG_LVL_INFO,"encrypt=");
    osapp_utils_log_hex_data(enc_output,16);
    LOG(LOG_LVL_INFO,"decrypt=");
    osapp_utils_log_hex_data(dec_output,16);
    
}






