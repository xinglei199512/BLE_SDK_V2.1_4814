#ifndef _AES_ECB_BLOCKED_H_
#define _AES_ECB_BLOCKED_H_
#include <stdint.h>



//change nvds_tag.c 
//DEFINE_TAG(NVDS_TAG_EXT_WAKEUP_TIME,{50}),
//to save operation time.




typedef enum
{
    AES_ECB_BLOCKED_ENCRYPT,
    AES_ECB_BLOCKED_DECRYPT,
} aes_operation_t;





/**
 ****************************************************************************************
 * @brief AES encrypt or decrypt in blocked mode.[big endian]
 *
 * @param[in] key     aes keys
 * @param[in] input   input data
 * @param[in] output  output data
 * @param[in] mode    AES_ECB_BLOCKED_ENCRYPT: input=plain data     , output=encrypted data      <p>
                      AES_ECB_BLOCKED_DECRYPT: input=encrypted data , output=plain data
 *
 * @return void 
 ****************************************************************************************
 */
void aes_ecb_bloked_operation(const uint8_t * key, const uint8_t * input, uint8_t * output , aes_operation_t mode);

/**
 ****************************************************************************************
 * @brief AES encrypt or decrypt in blocked mode.[little endian]
 ****************************************************************************************
 */
void aes_ecb_bloked_operation_rev(const uint8_t * key, const uint8_t * input, uint8_t * output , aes_operation_t mode);

#endif // End of _AES_ECB_BLOCKED_H_


