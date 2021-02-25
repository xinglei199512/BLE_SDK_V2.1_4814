#ifndef ADV_BEARER_RX_H_
#define ADV_BEARER_RX_H_
#include "network_pdu.h"


network_pdu_rx_t *network_pdu_alloc_and_fill(uint8_t *data,uint8_t length,uint8_t type,uint8_t from);
void mesh_msg_rx(network_pdu_rx_t *pdu);
void mesh_adv_rx(uint8_t ad_type,uint8_t *data,uint8_t length,ble_txrx_time_t rx_time,uint8_t rssi);
void gatt_mesh_msg_rx(uint8_t *data,uint8_t length,uint8_t type,uint8_t from);
void rx_pdu_buf_release(network_pdu_rx_t *ptr);


#endif


