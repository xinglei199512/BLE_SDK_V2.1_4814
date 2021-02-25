#include <string.h>
#include "co_math.h"
#include "co_endian.h"
#include "sdk_mesh_definitions.h"
#include "lower_tx_process.h"
#include "timers.h"
#include "reassembly.h"
#include "network_pdu.h"
#include "upper_pdu.h"
#include "network_tx_process.h"
#include "mesh_core_api.h"
#include "mesh_iv_operation_ex.h"
#include "log.h"
#include "network_keys_dm.h"

network_pdu_tx_t *upper_pdu_network_head_build(upper_pdu_tx_base_t *pdu,uint8_t nid,bool seq_auth_as_seq_num)
{
    uint32_t seq_num = seq_auth_as_seq_num ? pdu->seq_auth :  mesh_seqnum_alloc(1);
    network_pdu_packet_head_t network_head = {
        .nid = nid,
        .ivi = pdu->iv_index & 0x1,
        .ttl = pdu->ttl,
        .ctl = pdu->ctl,
        .seq_be = SEQ_ENDIAN_REVERSE(seq_num),
        .src_be = co_bswap16(pdu->src_addr),
        .dst_be = co_bswap16(pdu->dst_addr),
    };
    network_pdu_tx_t *tx = network_tx_pdu_head_build(&network_head);
    return tx;
}

network_pdu_tx_t *access_pdu_head_build(access_pdu_tx_t *access,uint8_t nid,bool seg,bool seq_auth_as_seq_num)
{
    network_pdu_tx_t *ptr = upper_pdu_network_head_build(&access->pdu_base,nid,seq_auth_as_seq_num);
    lower_pkt_head_u *head = (lower_pkt_head_u *)&ptr->src.lower_pdu;
    head->access.seg = seg ? 1 : 0;
    if(access->cryptic.appkey)
    {
        head->access.akf = 1;
        head->access.aid = access->cryptic.appkey_box->aid;
    }else
    {
        head->access.akf = 0;
        head->access.aid = 0;
    }
    return ptr;
}

network_pdu_tx_t *control_pdu_head_build(control_pdu_tx_t *ctrl,uint8_t nid,bool seg,bool seq_auth_as_seq_num)
{
    network_pdu_tx_t *ptr = upper_pdu_network_head_build(&ctrl->pdu_base,nid,seq_auth_as_seq_num);
    lower_pkt_head_u *head = (lower_pkt_head_u *)&ptr->src.lower_pdu;
    head->control.seg = seg ? 1 : 0;
    head->control.opcode = ctrl->opcode;
    return ptr;
}

network_pdu_tx_t *unseg_access_pdu_build(access_pdu_tx_t *access,uint8_t nid,uint8_t SegO,uint8_t SegN,bool seq_auth_as_seq_num)
{
    network_pdu_tx_t *ptr = access_pdu_head_build(access,nid,false,seq_auth_as_seq_num);
    memcpy(&ptr->src.lower_pdu[1],access->encrypted,access->pdu_base.total_length);
    ptr->src.lower_pdu_length = access->pdu_base.total_length + 1;
    return ptr;
}

static uint16_t seg_seq_zero_get(upper_pdu_tx_base_t *upper)
{
    uint32_t seq_auth = upper->seq_auth;
    return LSB(seq_auth,SEQZERO_LENGTH);
}

network_pdu_tx_t *seg_access_pdu_build(access_pdu_tx_t *access,uint8_t nid,uint8_t SegO,uint8_t SegN,bool seq_auth_as_seq_num)
{
    uint8_t segment_length  = 0;
    SegO = LSB(SegO,SEGO_LENGTH);
    SegN = LSB(SegN,SEGN_LENGTH);
    network_pdu_tx_t *ptr = access_pdu_head_build(access,nid,true,seq_auth_as_seq_num);
    uint8_t *seg_buf = &ptr->src.lower_pdu[1];
    uint16_t seq_zero = seg_seq_zero_get(&access->pdu_base);
    seg_buf[0] = access->szmic<<7 | seq_zero >> 6;
    seg_buf[1] = seq_zero<<2 | SegO >> 3;
    seg_buf[2] = SegO<<5 | SegN;
    segment_length = SegO<SegN ? SEGMENTED_ACCESS_MSG_MAX_LENGTH : (access->pdu_base.total_length - SEGMENTED_ACCESS_MSG_MAX_LENGTH*SegO);
    memcpy(&seg_buf[3],&access->encrypted[SEGMENTED_ACCESS_MSG_MAX_LENGTH*SegO],segment_length);
    ptr->src.lower_pdu_length = segment_length + 4;
    return ptr;
}

network_pdu_tx_t *unseg_control_pdu_build(control_pdu_tx_t *ctrl,uint8_t nid,uint8_t SegO,uint8_t SegN,bool seq_auth_as_seq_num)
{
    network_pdu_tx_t *ptr = control_pdu_head_build(ctrl,nid,false,seq_auth_as_seq_num);
    memcpy(&ptr->src.lower_pdu[1],ctrl->payload,ctrl->pdu_base.total_length);
    ptr->src.lower_pdu_length = ctrl->pdu_base.total_length + 1;
    return ptr;
}

network_pdu_tx_t *seg_control_pdu_build(control_pdu_tx_t *ctrl,uint8_t nid,uint8_t SegO,uint8_t SegN,bool seq_auth_as_seq_num)
{
    SegO = LSB(SegO,SEGO_LENGTH);
    SegN = LSB(SegN,SEGN_LENGTH);
    network_pdu_tx_t *ptr = control_pdu_head_build(ctrl,nid,true,seq_auth_as_seq_num);
    uint8_t *seg_buf = &ptr->src.lower_pdu[1];
    uint16_t seq_zero = seg_seq_zero_get(&ctrl->pdu_base);
    seg_buf[0] = seq_zero >> 6;
    seg_buf[1] = seq_zero<<2 | SegO >> 3;
    seg_buf[2] = SegO<<5 | SegN;
    uint8_t segment_length = SegO < SegN ? SEGMENTED_CONTROL_MSG_MAX_LENGTH : ctrl->pdu_base.total_length - SEGMENTED_CONTROL_MSG_MAX_LENGTH*SegO;
    memcpy(&seg_buf[3],&ctrl->payload[SEGMENTED_CONTROL_MSG_MAX_LENGTH*SegO],segment_length);
    ptr->src.lower_pdu_length = segment_length + 4;
    return ptr;
}

uint16_t relay_adv_tx_interval_get(uint16_t dst_addr)
{
    mesh_core_params_t param_relay_transmit;
    mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_RELAY_TRANSMIT   , &param_relay_transmit);
    return (param_relay_transmit.relay_transmit.interval_steps + 1) * 10;
}

uint16_t segment_ack_adv_tx_interval_get(uint16_t dst_addr)
{
    mesh_core_params_t param_network_transmit;
    mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT , &param_network_transmit);
    return (param_network_transmit.network_transmit.interval_steps + 1) * 10;
}

uint32_t segment_pkt_tx_start(uint16_t dst_addr,bool *local,bool *adv,uint32_t *gatt_mask,network_adv_tx_t *adv_tx)
{
    mesh_core_params_t param_network_transmit;
    mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT , &param_network_transmit);
    adv_tx->param.repeats = param_network_transmit.network_transmit.count;
    adv_tx->param.high_priority = false;
    return pdMS_TO_TICKS(rand()%SEGMENT_ACK_DELAY_RAND_MAX);
}

uint32_t relay_pkt_tx_start(uint16_t dst_addr,bool *local,bool *adv,uint32_t *gatt_mask,network_adv_tx_t *adv_tx)
{
    mesh_core_params_t param_relay_transmit;
    mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_RELAY_TRANSMIT   , &param_relay_transmit);
    adv_tx->param.repeats = param_relay_transmit.relay_transmit.count;
    adv_tx->param.high_priority = false;
    return pdMS_TO_TICKS(rand()%RELAY_DELAY_RAND_MAX);
}


void relay_pkt_send(network_pdu_decrypt_callback_param_t *param,uint8_t total_length)
{
    network_pdu_tx_t *tx = network_tx_pdu_head_build((network_pdu_packet_head_t *)param->decrypted.buf);
    tx->src.head.ttl -= 1;
    tx->src.lower_pdu_length = network_transport_pdu_length(total_length,tx->src.head.ctl);
    memcpy(tx->src.lower_pdu,&param->decrypted.buf[sizeof(network_pdu_packet_head_t)],tx->src.lower_pdu_length);
    tx->iv_index = param->iv_index;
    dm_netkey_get_netkey_tx_credentials(param->netkey,&tx->netkey_credentials);
    tx->pkt_type = RELAY_ADV_PKT;
    network_tx_process_start(tx);
}

void segment_ack_send(segment_ack_param_t *ack)
{
    uint32_t iv_index = mesh_tx_iv_index_get();
    uint32_t seq_num = mesh_seqnum_alloc(1);
    uint32_t block_be = co_bswap32(ack->block_ack);
    network_pdu_packet_head_t head = 
    {
        .ivi = iv_index & 0x1,
        .nid = ack->net_security->nid,
        .ctl = 1,
        .ttl = ack->ttl,
        .seq_be = SEQ_ENDIAN_REVERSE(seq_num),
        .src_be = co_bswap16(ack->src_addr),
        .dst_be = co_bswap16(ack->dst_addr),
    };
    network_pdu_tx_t *msg = network_tx_pdu_head_build(&head);
    uint8_t *pdu = msg->src.lower_pdu;
    pdu[0] = 0;
    pdu[1] = (ack->obo<<7 | ack->seq_zero>>6) &0xff;
    pdu[2] = ack->seq_zero<<2 & 0xff;
    memcpy(&pdu[3],&block_be,sizeof(uint32_t));
    msg->src.lower_pdu_length = SEGMENT_ACK_MSG_LENGTH;
    msg->iv_index = iv_index;
    msg->netkey_credentials = ack->net_security;
    msg->pkt_type = SEGMENT_ACK_ADV_PKT;
    network_tx_process_start(msg);
}


