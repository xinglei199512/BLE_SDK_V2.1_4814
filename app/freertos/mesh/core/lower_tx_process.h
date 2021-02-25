#ifndef LOWER_TX_PROCESS_H_
#define LOWER_TX_PROCESS_H_
#include "network_pdu.h"
#include "upper_pdu.h"
#include "adv_bearer_tx.h"
#include "network_pdu_decrypt.h"

#define SEGMENT_ACK_DELAY_RAND_MAX 30
#define RELAY_DELAY_RAND_MAX 30

#define SEGMENT_ACK_MSG_LENGTH                      7


network_pdu_tx_t *seg_control_pdu_build(control_pdu_tx_t *ctrl,uint8_t nid,uint8_t SegO,uint8_t SegN,bool seq_auth_as_seq_num);
network_pdu_tx_t *unseg_control_pdu_build(control_pdu_tx_t *ctrl,uint8_t nid,uint8_t SegO,uint8_t SegN,bool seq_auth_as_seq_num);
network_pdu_tx_t *seg_access_pdu_build(access_pdu_tx_t *access,uint8_t nid,uint8_t SegO,uint8_t SegN,bool seq_auth_as_seq_num);
network_pdu_tx_t *unseg_access_pdu_build(access_pdu_tx_t *access,uint8_t nid,uint8_t SegO,uint8_t SegN,bool seq_auth_as_seq_num);

uint16_t relay_adv_tx_interval_get(uint16_t dst_addr);
uint16_t segment_ack_adv_tx_interval_get(uint16_t dst_addr);
uint32_t segment_pkt_tx_start(uint16_t dst_addr,bool *local,bool *adv,uint32_t *gatt_mask,network_adv_tx_t *adv_tx);
uint32_t relay_pkt_tx_start(uint16_t dst_addr,bool *local,bool *adv,uint32_t *gatt_mask,network_adv_tx_t *adv_tx);

void relay_pkt_send(network_pdu_decrypt_callback_param_t *param,uint8_t total_length);
void segment_ack_send(segment_ack_param_t *ack);


#endif
