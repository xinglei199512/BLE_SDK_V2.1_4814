#ifndef AES_CCM_CMAC_H_
#define AES_CCM_CMAC_H_
#include <stdint.h>
#include <stdbool.h> 

#define NONCE_LENGTH                        13

#define MAX_BLOCK_NUM               26    // > 384byte

#define AES_BLOCK_SIZE                      16
#define MSG_LENGTH_FIELD_SIZE               (AES_BLOCK_SIZE -NONCE_LENGTH - 1)
#define ADDITIONAL_DATA_LENGTH_FIELD_SIZE   2


enum Decryption_Result{
  DECRYPT_SUCCESSFUL,
  AUTH_FAILED,
};

typedef struct
{
    uint8_t const *key;
    uint8_t const *nonce;
    uint8_t const *msg;
    uint8_t const *additional_data;
    uint8_t *rslt;
    uint8_t msg_length;
    uint8_t mic_length;
    uint8_t additional_data_length;
}aes_ccm_param_t;

typedef struct
{
    uint8_t const *k;
    uint8_t const *m;
    uint8_t *rslt;
    uint8_t length;
}aes_cmac_param_t;

typedef enum
{
    CCM_ENCRYPT,
    CCM_DECRYPT,
    CMAC_CALC,
}aes_ccm_cmac_operation_t;

typedef struct
{
    union
    {
        aes_ccm_param_t ccm;
        aes_cmac_param_t cmac;
    }param;
    aes_ccm_cmac_operation_t op_type;
}ccm_cmac_buf_t;

void ccm_cmac_start(ccm_cmac_buf_t *buf,void (*callback)(ccm_cmac_buf_t *,void *,uint8_t));
bool  security_is_busy(void);

#endif
