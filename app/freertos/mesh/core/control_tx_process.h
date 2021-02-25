#ifndef CONTROL_TX_PROCESS_H_
#define CONTROL_TX_PROCESS_H_
#include "upper_pdu.h"
#include "virt_addr_mngt.h"
#include "upper_tx_process.h"

typedef struct{
    ble_txrx_time_t *expected_tx_time;
    security_credentials_t *netkey_credentials;
    uint16_t dst_addr;
    uint16_t length;
    uint8_t opcode;
    uint8_t ttl;
    bool high_priority;
}control_msg_tx_param_t;

void control_tx_complete(control_pdu_tx_t *tx,uint8_t status);
control_pdu_tx_t *control_tx_pdu_alloc(uint16_t pdu_length,bool seg, uint8_t opcode);
void control_set_tx_dst_addr(control_pdu_tx_t *ptr,virt_addr_mngt_t *virt,uint16_t dst_addr);
void control_send(control_pdu_tx_t *ptr);
control_pdu_tx_t *control_unseg_msg_build(control_msg_tx_param_t *param,void (*callback)(void *,uint8_t));



#endif
