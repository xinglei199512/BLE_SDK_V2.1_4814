#ifndef UPPER_RX_PROCESS_H_
#define UPPER_RX_PROCESS_H_
#include "upper_pdu.h"
#include "network_pdu_decrypt.h"

void upper_pdu_build_and_dispatch(network_pdu_decrypt_callback_param_t *param,
    uint8_t from,uint32_t seq_auth,uint8_t *data,uint16_t total_length,ble_txrx_time_t rx_time,uint8_t rssi);
void upper_rx_pdu_free(upper_pdu_rx_t *pdu);

#endif
