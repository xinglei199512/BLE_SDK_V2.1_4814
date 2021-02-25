#ifndef NETWORK_TX_PROCESS_H_
#define NETWORK_TX_PROCESS_H_
#include "network_pdu.h"
#include "adv_bearer_tx.h"
#include "timer_wrapper.h"


void network_tx_process_start(network_pdu_tx_t *pdu);
network_pdu_tx_t *network_tx_pdu_head_build(network_pdu_packet_head_t *head);
void network_tx_on_adv_start_callback(mesh_adv_tx_t *adv);
bool network_adv_tx_cancel(network_adv_tx_t *ptr);
network_pdu_tx_t *tx_pdu_buf_alloc(void);
void tx_pdu_buf_release(network_pdu_tx_t *ptr);
void network_adv_tx_buf_retain(network_adv_tx_t *ptr);
bool network_adv_tx_buf_release(network_adv_tx_t *ptr);
void network_adv_retransmit_for_friend_q_element(network_adv_tx_t *adv,uint32_t delay_ticks);



#endif
