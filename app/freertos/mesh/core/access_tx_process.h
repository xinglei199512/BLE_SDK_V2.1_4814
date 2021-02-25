#ifndef ACCESS_TX_PROCESS_H_
#define ACCESS_TX_PROCESS_H_
#include "virt_addr_mngt.h"
#include "upper_tx_process.h"

typedef struct
{
    uint32_t  opcode;
    union{
        app_key_t *app_key;
        uint16_t netkey_idx;
    }key;
    union{
        virt_addr_mngt_t *virt;
        uint16_t addr;
    }dst_addr;
    ble_txrx_time_t *rx_time;
    uint16_t *interval_ms;
    uint8_t *repeats;
    uint8_t *ttl;
    uint16_t pdu_length;
    uint16_t src_addr;
    uint8_t seg:1,
            szmic:1,
            credential_flag : 1,
            akf : 1,
            virt: 1 ;
}model_tx_msg_param_t;

access_pdu_tx_t *access_model_pkt_build_fill(model_tx_msg_param_t *param,void (*callback)(void *,uint8_t),uint8_t *data);
access_pdu_tx_t *access_model_msg_build(model_tx_msg_param_t *param,void (*callback)(void *,uint8_t));
uint8_t *access_model_tx_payload_ptr(access_pdu_tx_t *ptr,uint32_t opcode);
void access_send(access_pdu_tx_t *ptr);
security_credentials_t *access_get_netkey_credentials_by_idx(uint16_t idx,uint8_t credential_flag);

void access_tx_complete(access_pdu_tx_t *tx,uint8_t status);
void access_tx_process_start(upper_tx_env_t *ptr);

#endif
