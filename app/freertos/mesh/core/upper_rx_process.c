#define LOG_TAG        "upper_rx_process.c"
#define LOG_LVL        LVL_DBG
#include "bx_log.h"


#include"co_endian.h"
#include"mesh_env.h"
#include "upper_rx_process.h"
#include "network_pdu_decrypt.h"
#include "static_buffer.h"
#include "control_rx_process.h"
#include "access_rx_process.h"
#include "stack_mem_cfg.h"

#define SZMIC_MASK                                  0x80
#define SZMIC_OFFSET                                7

DEF_ARRAY_BUF(upper_pdu_rx_buf,upper_pdu_rx_t,UPPER_PDU_RX_BUF_SIZE);

upper_pdu_rx_t *upper_pdu_rx_buf_alloc()
{
    return array_buf_alloc(&upper_pdu_rx_buf);
}

void upper_pdu_rx_buf_release(upper_pdu_rx_t *ptr)
{
    array_buf_release(&upper_pdu_rx_buf,ptr);
}


void upper_rx_pdu_free(upper_pdu_rx_t *pdu)
{
    mesh_free(pdu->src);
    upper_pdu_rx_buf_release(pdu);
}

void upper_pdu_build_and_dispatch(network_pdu_decrypt_callback_param_t *param,uint8_t from,uint32_t seq_auth,uint8_t *data,uint16_t total_length,ble_txrx_time_t rx_time,uint8_t rssi)
{
    LOG_D("%s",__func__);
    uint8_t *lower_pdu = &param->decrypted.buf[ENCRYPTED_DATA_OFFSET + 2];
    lower_pkt_head_u head = *(lower_pkt_head_u *)lower_pdu;
    upper_pdu_rx_t *upper_pdu = upper_pdu_rx_buf_alloc();
    network_pdu_packet_head_t *pkt = &param->decrypted.pkt;
    uint16_t src_addr = co_bswap16(pkt->src_be);
    uint16_t dst_addr = co_bswap16(pkt->dst_be);
    upper_pdu->rx_time = rx_time;
    upper_pdu->iv_index = param->iv_index;
    upper_pdu->netkey = param->netkey;
    upper_pdu->net_security = param->net_security;
    upper_pdu->src_addr = src_addr;
    upper_pdu->dst_addr = dst_addr;
    upper_pdu->from = from;
    upper_pdu->rssi = rssi;
    upper_pdu->ttl = pkt->ttl;
    upper_pdu->ctl = pkt->ctl;
    if(pkt->ctl)
    {
        upper_pdu->head.control.opcode = head.control.opcode;
    }else
    {
        upper_pdu->head.access.aid = head.access.aid;
        upper_pdu->head.access.akf = head.access.akf;
        if(head.head.seg == 1)
        {
            upper_pdu->head.access.szmic = (lower_pdu[1]& SZMIC_MASK)>>SZMIC_OFFSET;
        }
        else
        {
            upper_pdu->head.access.szmic = 0;
        }
    }
    upper_pdu->seq_auth = seq_auth;
    upper_pdu->src = data;
    upper_pdu->total_length = total_length;
    if(upper_pdu->ctl)
    {
        control_pdu_rx_process(upper_pdu);
    }else
    {
        access_rx_process_start(upper_pdu);
    }
}

