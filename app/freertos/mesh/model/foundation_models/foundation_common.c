
#include "foundation_common.h"
#include "access_tx_process.h"
#include "foundation_msg_handler.h"
#include "log.h"
uint16_t dst_addrs=0;

void fd_model_two_octets_status_tx(uint16_t netkey_idx,uint16_t dst_addr,ble_txrx_time_t *rx_time,void (*cb)(void *,uint8_t),
    uint8_t *payload,uint8_t length,uint16_t opcode_offset)
{
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(FOUNDATION_MODELS_TWO_OCTETS_OPCODE_OFFSET,opcode_offset);
    tx_param.key.netkey_idx = netkey_idx;
    tx_param.dst_addr.addr = dst_addr;
	  dst_addrs=dst_addr;
    tx_param.rx_time = rx_time;
    tx_param.interval_ms = NULL;
    tx_param.repeats = NULL;
    tx_param.ttl = NULL;
    tx_param.pdu_length = length;
    tx_param.src_addr = mesh_node_get_primary_element_addr();
    tx_param.seg = 1;
    tx_param.szmic = 0;
    tx_param.credential_flag = 0;
    tx_param.akf = 0;
    tx_param.virt = 0;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,cb,payload);
    BX_ASSERT(ptr);
    access_send(ptr);
    LOG(3,"fd_model_two_octets_status_response_tx\n");
}

void fd_model_two_octets_status_response_tx(access_pdu_rx_t *rx_pdu,void (*cb)(void *,uint8_t),
    uint8_t *payload,uint8_t length,uint16_t opcode_offset)
{
    uint16_t netkey_idx = access_get_pdu_netkey_global_index(rx_pdu);
    uint16_t dst_addr  = access_get_pdu_src_addr(rx_pdu);
    ble_txrx_time_t rx_time = access_rx_get_rx_time(rx_pdu);
    fd_model_two_octets_status_tx(netkey_idx,dst_addr,&rx_time,cb,payload,length,opcode_offset);
}
