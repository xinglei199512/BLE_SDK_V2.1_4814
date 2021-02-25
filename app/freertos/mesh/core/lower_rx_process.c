#define LOG_TAG        "lower_rx_process.c"
#define LOG_LVL        LVL_DBG
#include "bx_log.h"


#include <string.h>
#include "lower_rx_process.h"
#include "reassembly.h"
#include "network_pdu.h"
#include "upper_pdu.h"
#include "upper_rx_process.h"
#include "upper_tx_process.h"
#include "co_math.h"
#include "co_endian.h"
#include "mesh_env.h"
#include "mesh_iv_operation_ex.h"
#include "network_tx_process.h"
#include "adv_bearer_tx.h"
#include "log.h"
#include "lower_tx_process.h"
#include "mesh_queued_msg.h"
#include "os_wrapper.h"
#include "config_server.h"
#include "friend.h"
#include "low_power.h"


#define INCOMPLETE_BLOCK_TIME 10000
#define LOWER_RX_ENV_BUF_SIZE 20


static struct co_list lower_rx_inactive_list;
DEF_ARRAY_BUF(lower_rx_env, lower_rx_env_t, LOWER_RX_ENV_BUF_SIZE);


uint16_t lower_pdu_seq_zero_get(uint8_t *lower_pdu)
{
    return ((lower_pdu[1] << 16 |lower_pdu[2] <<8) & SEQZERO_MASK)>>SEQZERO_OFFSET;
}

static bool lower_pdu_integrity_check(uint32_t block_ack,uint8_t SegN)
{
    return (1<<(SegN+ 1)) - 1 ==  block_ack;
}

static void seg_ack_send_by_reassembly_env(lower_rx_env_t *env)
{
    segment_ack_param_t ack = 
    {
        .block_ack = env->reassembly->block_ack,
        .seq_zero = LSB(env->current_seq_auth,SEQZERO_LENGTH),
        .ttl = config_server_get_default_ttl(),
        .src_addr = env->reassembly->dst_addr,
        .dst_addr = env->src_addr,
    };
    if(env->reassembly->friend)
    {
        ack.obo = 1;
        struct co_list_hdr *hdr = co_list_pick(&env->reassembly->friend->seg_list);
        BX_ASSERT(hdr);
        friend_q_element_t *elem = CONTAINER_OF(hdr, friend_q_element_t, hdr);
        ack.netkey = NULL;
        ack.net_security = elem->tx_pdu->netkey_credentials;
    }else
    {
        ack.obo = 0;
        ack.netkey = env->reassembly->local->netkey;
        ack.net_security = env->reassembly->local->net_security;
    }
    //LOG(3, "seg_ack_send_by_reassembly_env block_ack:%x env:%p\n", ack.block_ack, env);
    segment_ack_send(&ack);
}

void ack_timer_expire_handler(lower_rx_env_t *env)
{
    if(env->reassembly)
    {
        LOG(3, "ack_timer_expire_handler env:%p\n", env);
        seg_ack_send_by_reassembly_env(env);
    }else
    {
        //LOG
    }
}

static void reassembly_storage_env_cleanup(reassembly_env_t *env,bool need_free_data)
{
    if(env->friend)
    {
        friend_seg_list_cleanup(env->friend);
        friend_env_release(env->friend);
        env->friend = NULL;
    }
    if(env->local && need_free_data)
    {
        mesh_free(env->local->data);
        env->local->data = NULL;
    }
}

void incomplete_timer_expire_handler(lower_rx_env_t *env)
{

    if(env->reassembly)
    {
        if(env->state == INCOMPLETE_TIMEOUT)
        {
            env->state = SEG_PDU_RX;
            bool released = reassembly_env_release(env->reassembly);
            BX_ASSERT(released);
            env->reassembly = NULL;
        }else
        {
            env->state = INCOMPLETE_TIMEOUT;
            if(IS_UNICAST_ADDR(env->reassembly->dst_addr))
            {
                mesh_timer_cancel(env->reassembly->timer.acknowledgment);
            }
            reassembly_storage_env_cleanup(env->reassembly,true);
            mesh_timer_change_period(env->reassembly->timer.incomplete,pdMS_TO_TICKS(INCOMPLETE_BLOCK_TIME));
        }
    }else
    {
        //LOG
    }
}

void ack_timer_callback(mesh_timer_t timer)
{
    lower_rx_env_t *env = mesh_timer_get_associated_data(timer);
    mesh_queued_msg_send((void (*)(void *))ack_timer_expire_handler,env);
}

void incomplete_timer_callback(mesh_timer_t timer)
{
    lower_rx_env_t *env = mesh_timer_get_associated_data(timer);
    mesh_queued_msg_send((void (*)(void *))incomplete_timer_expire_handler,env);
}

static int seq_auth_cmp(lower_rx_env_t *env,uint32_t rx_seq_auth,uint32_t iv_index)
{
    if(env->iv_index == iv_index)
    {
        if(env->current_seq_auth == rx_seq_auth)
        {
            return 0;
        }else if(env->current_seq_auth < rx_seq_auth)
        {
            return -1; //rx_seq_auth is new
        }else
        {
            return 1; //rx_seq_auth is old
        }
    }else if(env->iv_index > iv_index)
    {
          return 1;       // rx_seq_auth is old
    }
    else
    {
          return -1;       // rx_seq_auth is new
    }
}

uint32_t seq_auth_lsb(uint32_t current_seq_num,uint16_t seqzero)
{
    int32_t seq_auth1=0,seq_auth2=0;
    seq_auth1=((current_seq_num-0x1FFF)&0xFFFFE000) | seqzero;//sub
    seq_auth2=((current_seq_num-0x0000)&0xFFFFE000) | seqzero;//no sub
    if((seq_auth1 >= (int32_t)(current_seq_num-0x1FFF)) && (seq_auth1 <= current_seq_num))//judge overflow
    {
        return seq_auth1;
    }
    else if((seq_auth2 >= (int32_t)(current_seq_num-0x1FFF)) && (seq_auth2 <= current_seq_num))
    {
        return seq_auth2;
    }
    return 0;
}

static void incomplete_timer_start(reassembly_env_t *env)
{
    mesh_timer_start(env->timer.incomplete);
}

static void ack_timer_start(reassembly_env_t *env,uint8_t ttl)
{
    mesh_timer_change_period(env->timer.acknowledgment, pdMS_TO_TICKS(ACK_TIMER(ttl)));
}

static lower_rx_env_t *lower_rx_env_alloc(uint16_t src_addr)
{
    lower_rx_env_t *ptr = array_buf_alloc(&lower_rx_env);
    if(ptr)
    {
        ptr->src_addr = src_addr;
    }
    return ptr;
}

static bool lower_rx_env_release(lower_rx_env_t *ptr)
{
    return array_buf_release(&lower_rx_env,ptr);
}

static lower_rx_env_t *lower_rx_env_search(uint16_t src_addr)
{
    lower_rx_env_t *ptr;
    FOR_EACH_ALLOCATED_ITEM(lower_rx_env, ptr,
        if(ptr->src_addr == src_addr)
        {
            return ptr;
        }
    )
    return NULL;
}

static uint16_t lower_rx_env_available_size()
{
    return array_buf_available_size(&lower_rx_env);
}

static void reassembly_env_cleanup(reassembly_env_t *env,bool need_free_data)
{
    if(IS_UNICAST_ADDR(env->dst_addr))
    {
        mesh_timer_cancel(env->timer.acknowledgment);
    }
    mesh_timer_cancel(env->timer.incomplete);
    reassembly_storage_env_cleanup(env,need_free_data);
    bool released = reassembly_env_release(env);
    BX_ASSERT(released);
}

static bool is_seg_ack(lower_pkt_head_u head,uint8_t ctl)
{
    if(ctl == 1 && head.control.opcode == 0)
    {
        return true;
    }else
    {
        return false;
    }
}

static bool is_friend_update(lower_pkt_head_u head,uint8_t ctl)
{
    if(ctl == 1 && head.control.opcode == FRIEND_UPDATE)
    {
        return true;
    }else
    {
        return false;
    }
    
}

void lower_transport_rx(network_pdu_decrypt_callback_param_t *param,uint8_t total_length,
    uint8_t from,ble_txrx_time_t rx_time,uint8_t rssi,friend_list_t *friend,bool local)
{
    LOG_D("%s",__func__);
    uint8_t *lower_pdu = &param->decrypted.buf[ENCRYPTED_DATA_OFFSET + 2];
    lower_pkt_head_u head = *(lower_pkt_head_u *)lower_pdu;
    network_pdu_packet_head_t *pkt = &param->decrypted.pkt;
    uint32_t seq_num = SEQ_ENDIAN_REVERSE(pkt->seq_be);
    uint16_t src_addr = co_bswap16(pkt->src_be);
    uint16_t dst_addr = co_bswap16(pkt->dst_be);
    uint8_t payload_length = network_transport_pdu_length(total_length,pkt->ctl)  - (head.head.seg ? SEGMENTED_PAYLOAD_OFFSET : 1);
    bool low_power_mode = low_power_mode_status_get();
    uint16_t seqzero;
    uint8_t SegN;
    uint8_t SegO;
    uint8_t segment_payload_max_length;
    uint32_t rx_seq_auth;
    if(head.head.seg)
    {
        seqzero = lower_pdu_seq_zero_get(lower_pdu);
        SegN = (lower_pdu[3]&SEGN_MASK)>>SEGN_OFFSET;
        SegO = ((lower_pdu[2]<<8|lower_pdu[3])&SEGO_MASK)>>SEGO_OFFSET;
        segment_payload_max_length = pkt->ctl? SEGMENTED_CONTROL_MSG_MAX_LENGTH: SEGMENTED_ACCESS_MSG_MAX_LENGTH;
        rx_seq_auth = seq_auth_lsb(seq_num,seqzero);
        LOG(LOG_LVL_WARN,"seg auth: %d - %d - %d\n",seqzero,rx_seq_auth,seq_num);
    }else
    {
        if(is_seg_ack(head,pkt->ctl))
        {
            seqzero = lower_pdu_seq_zero_get(lower_pdu);
        }
        rx_seq_auth = seq_num;
    }
    lower_rx_env_t *env = lower_rx_env_search(src_addr);
    if(env == NULL)
    {
        if(lower_rx_env_available_size()==0)
        {
            struct co_list_hdr *hdr = co_list_pop_front(&lower_rx_inactive_list);
            if(hdr)
            {
                lower_rx_env_t *ptr = CONTAINER_OF(hdr,lower_rx_env_t,hdr);
                lower_rx_env_release(ptr);
            }else
            {
                LOG(LOG_LVL_WARN,"all lower rx env in use\n");
                return;
            }
        }
        env = lower_rx_env_alloc(src_addr);
        BX_ASSERT(env);
    }else
    {
        int seq_auth_cmp_rslt = seq_auth_cmp(env,rx_seq_auth,param->iv_index);
        if(seq_auth_cmp_rslt == 0)
        {
            if(is_seg_ack(head, pkt->ctl) == false)
            {
                if(env->state == INCOMPLETE_TIMEOUT)
                {
                    LOG(LOG_LVL_WARN,"incomplete block\n");
                    return;
                }else if(env->state == COMPLETE_PDU_RX)
                {
                    if(head.head.seg && IS_UNICAST_ADDR(dst_addr))
                    {
                        segment_ack_param_t ack = 
                        {
                            .netkey = param->netkey,
                            .net_security = param->net_security,
                            .block_ack = (1<< SegN + 1) - 1,
                            .seq_zero = LSB(rx_seq_auth,SEQZERO_LENGTH),
                            .ttl = config_server_get_default_ttl(),
                            .obo = friend? 1 : 0,
                            .src_addr = dst_addr,
                            .dst_addr = src_addr,
                            };
                            segment_ack_send(&ack);
                    }
                    LOG(LOG_LVL_WARN,"complete pdu already received\n");
                    return;
                }
            }
        }else if(seq_auth_cmp_rslt>0)
        {
            LOG(LOG_LVL_WARN,"seg replay protect\n");
            return;
        }else
        {
            if(env->reassembly)
            {
                reassembly_env_cleanup(env->reassembly,true);
                env->reassembly = NULL;
            }
        }
    }
    if(!is_seg_ack(head,pkt->ctl))
    {
        env->current_seq_auth = rx_seq_auth;
        env->iv_index = param->iv_index;
    }
    if(head.head.seg)
    {
        env->state = SEG_PDU_RX;
        if(env->reassembly == NULL)
        {
            env->reassembly = reassembly_env_alloc(local,env);
            if(env->reassembly == NULL)
            {
                if(IS_UNICAST_ADDR(dst_addr))
                {
                    segment_ack_param_t ack = 
                    {
                        .netkey = param->netkey,
                        .net_security = param->net_security,
                        .block_ack = 0,
                        .seq_zero = LSB(rx_seq_auth,SEQZERO_LENGTH),
                        .ttl = config_server_get_default_ttl(),
                        .obo = friend?1:0,
                        .src_addr = dst_addr,
                        .dst_addr = src_addr,
                    };
                    segment_ack_send(&ack);
                }
                LOG(LOG_LVL_WARN,"no available reassembly env\n");
                return;
            }
            env->reassembly->block_ack = 0;
            if(friend)
            {
                friend_env_retain(friend);
                env->reassembly->friend = friend;
                co_list_init(&friend->seg_list);
            }
            if(local)
            {
                uint16_t payload_max_length = segment_payload_max_length*(SegN+1);
                env->reassembly->local->data = mesh_alloc(payload_max_length);
                BX_ASSERT(env->reassembly->local->data);
                env->reassembly->local->netkey = param->netkey;
                env->reassembly->local->net_security = param->net_security;
            }
            env->reassembly->dst_addr = dst_addr;
            if(low_power_mode == false)
            {
                incomplete_timer_start(env->reassembly);
                if(IS_UNICAST_ADDR(dst_addr))
                {
                    ack_timer_start(env->reassembly, pkt->ttl);
                }
            }
        }
        if(SegO == SegN)
        {
            env->reassembly->last_seg_length = payload_length;
        }
        env->reassembly->block_ack |= 1<<SegO;
        if(env->reassembly->friend)
        {
            alloc_friend_q_element_and_push_to_list(env->reassembly->friend,&param->decrypted,total_length,&env->reassembly->friend->seg_list);
        }
        if(env->reassembly->local)
        {
            memcpy(env->reassembly->local->data + segment_payload_max_length*SegO,&lower_pdu[SEGMENTED_PAYLOAD_OFFSET],payload_length);
        }
        if(lower_pdu_integrity_check(env->reassembly->block_ack, SegN))
        {
            env->state = COMPLETE_PDU_RX;
            co_list_push_back(&lower_rx_inactive_list,&env->hdr);
            if(low_power_mode == false)
            {
                if(IS_UNICAST_ADDR(dst_addr))
                {
                    seg_ack_send_by_reassembly_env(env);
                }
            }
            if(env->reassembly->friend)
            {
                while(1)
                {
                    struct co_list_hdr *hdr = co_list_pop_front(&env->reassembly->friend->seg_list);
                    if(hdr==NULL)
                    {
                        break;
                    }
                    co_list_push_back(&env->reassembly->friend->msg_list, hdr);
                }
            }
            if(env->reassembly->local)
            {
                upper_pdu_build_and_dispatch(param,from,rx_seq_auth,env->reassembly->local->data,
                    segment_payload_max_length*SegN + env->reassembly->last_seg_length,rx_time,rssi);
            }
            reassembly_env_cleanup(env->reassembly,false);
            env->reassembly = NULL;
        }else
        {
            if(low_power_mode == false)
            {
                if(IS_UNICAST_ADDR(dst_addr))
                {
                    if(mesh_timer_active(env->reassembly->timer.acknowledgment)==false)
                    {
                        mesh_timer_reset(env->reassembly->timer.acknowledgment);
                    }
                }
                mesh_timer_active_then_reset(env->reassembly->timer.incomplete);
            }
        }
    }else
    {
        co_list_push_back(&lower_rx_inactive_list,&env->hdr);
        if(is_seg_ack(head,pkt->ctl))
        {
            if(friend)
            {
                friend_q_element_t *seg_ack = friend_q_seg_ack_search(friend, src_addr, dst_addr,seqzero);
                if(seg_ack)
                {
                    friend_q_remove(friend, seg_ack);
                    friend_q_element_env_free(&friend->queue,seg_ack);
                }
                alloc_friend_q_element_and_push_to_list(friend,&param->decrypted,total_length,&friend->msg_list);
            }else
            {
                uint32_t block_ack = lower_pdu[3]<<24 | lower_pdu[4]<<16 | lower_pdu[5]<<8 | lower_pdu[6];
                segment_ack_rx(src_addr,seqzero,block_ack);
            }
        }else
        {
            env->state = COMPLETE_PDU_RX;
            if(friend)
            {
                alloc_friend_q_element_and_push_to_list(friend,&param->decrypted,total_length,&friend->msg_list);
            }else
            {
                uint8_t *data = mesh_alloc(payload_length);
                BX_ASSERT(data);
                memcpy(data,&lower_pdu[1],payload_length);
                upper_pdu_build_and_dispatch(param,from,seq_num,data,payload_length,rx_time,rssi);
            }
        }
    }
    low_power_mode = low_power_mode_status_get();
    if(low_power_mode)
    {
        bool is_friend_update_pkt = is_friend_update(head,pkt->ctl);
        low_power_toggle_fsn();
        lpn_schedule_at_pkt_rx(is_friend_update_pkt);
    }
}


void lower_rx_env_reassembly_cleanup()
{
    lower_rx_env_t *ptr;
    FOR_EACH_ALLOCATED_ITEM(lower_rx_env, ptr,
        if(ptr->reassembly)
        {
            reassembly_env_cleanup(ptr->reassembly,true);
            ptr->reassembly = NULL;
        }
    )
}



