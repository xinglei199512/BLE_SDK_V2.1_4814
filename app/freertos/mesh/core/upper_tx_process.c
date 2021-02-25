#include <string.h>
#include "upper_tx_process.h"
#include "static_buffer.h"
#include "co_math.h"
#include "log.h"
#include "queued_async_framework.h"
#include "network_pdu_decrypt.h"
#include "network_tx_process.h"
#include "mesh_queued_msg.h"
#include "mesh_env.h"
#include "app_keys_dm.h"
#include "access_tx_process.h"
#include "control_tx_process.h"
#include "app_keys_dm.h"
#include "adv_bearer_tx.h"
#include "lower_tx_process.h"
#include "stack_mem_cfg.h"
#include "mesh_iv_operation_ex.h"
#include "os_wrapper.h"
#include "osapp_utils.h"
#include "task_init.h"
#include "low_power.h"

#ifndef SEGMENTED_PDU_RETRANSMIT_MAX_TIMES_UNICAST
#define SEGMENTED_PDU_RETRANSMIT_MAX_TIMES_UNICAST 2
#endif
#ifndef SEGMENTED_PDU_RETRANSMIT_MAX_TIMES_VIRTUAL_GROUP
#define SEGMENTED_PDU_RETRANSMIT_MAX_TIMES_VIRTUAL_GROUP 3
#endif

typedef network_pdu_tx_t *(*lower_pdu_build_fn_t)(void * msg,uint8_t nid,uint8_t SegO,uint8_t SegN,bool seq_auth_as_seq_num);

enum upper_adv_tx_pkt_states{
    PKT_IDLE,
    PKT_BUILD_WITH_SEQ_AUTH,
    PKT_BUILD_NORMAL,
    PKT_BUILT_COMPLETE,
    PKT_ADV_START,
    PKT_ADV_INTERVAL,
    PKT_CANCEL,
    PKT_DONE,
};

DEF_ARRAY_BUF(upper_tx_env_buf, upper_tx_env_t, UPPER_TX_ENV_BUF_SIZE);
DEF_ARRAY_BUF(upper_tx_mngt_buf,upper_tx_mngt_t,UPPER_TX_MNGT_BUF_SIZE);

static void cancel_ongoing_tx_pkt(upper_tx_env_t *env);
static void segment_retransmission_control_virtual_group(upper_tx_env_t *env);
static bool upper_adv_tx_build_next_pkt(upper_tx_mngt_t *mngt,upper_pdu_tx_base_t *pdu);


static upper_tx_env_t *upper_tx_env_search(uint16_t dst_addr,uint8_t ctl,upper_pdu_tx_base_t **current)
{
    upper_tx_env_t *env;
    FOR_EACH_ALLOCATED_ITEM(upper_tx_env_buf, env, 
        struct co_list_hdr *hdr = co_list_pick(&env->tx_list);
        upper_pdu_tx_base_t *pdu = CONTAINER_OF(hdr, upper_pdu_tx_base_t, hdr);
        if(pdu->dst_addr == dst_addr && pdu->ctl == ctl)
        {
            if(current)
            {
                *current = pdu;
            }
            return env;
        }
    );
    return NULL;
}

static uint8_t upper_tx_pdu_segn_get(uint16_t total_length,uint8_t ctl)
{
    return CEILING(total_length,ctl ? SEGMENTED_CONTROL_MSG_MAX_LENGTH : SEGMENTED_ACCESS_MSG_MAX_LENGTH) -1;
}

static void segment_tx_timer_callback(mesh_timer_t timer)
{
    upper_tx_env_t *env = mesh_timer_get_associated_data(timer);
    mesh_queued_msg_send((void (*)(void *))cancel_ongoing_tx_pkt,env);
}

static bool segment_tx_timer_check(upper_pdu_tx_base_t *pdu)
{
    return pdu->seg && IS_UNICAST_ADDR(pdu->dst_addr);
}

static upper_tx_mngt_t *upper_tx_mngt_alloc(upper_pdu_tx_base_t *pdu,upper_tx_env_t *env)
{
    upper_tx_mngt_t * mngt = array_buf_calloc(&upper_tx_mngt_buf);
    if(mngt)
    {
        mngt->SegN = pdu->seg ? upper_tx_pdu_segn_get(pdu->total_length,pdu->ctl) : 0;
        if(segment_tx_timer_check(pdu))
        {
            mngt->segment_tx_timer = mesh_timer_create("seg_tx",1,false,env,segment_tx_timer_callback);
        }else
        {
            mngt->segment_tx_timer = NULL;
        }
        mngt->adv_tx = mesh_alloc(sizeof(upper_adv_tx_env_t)*(mngt->SegN + 1));
        mngt->block_ack = 0;
        mngt->tx_cnt = 0;
        mngt->ack_received = false;
        mngt->need_resched = false;
    }
    return mngt;
}

static void upper_tx_adv_pkt_state_setup(upper_tx_mngt_t *mngt,bool retransmit)
{
    uint8_t i;
    if((mngt->block_ack&0x1)==0)
    {
        mngt->adv_tx[0].adv = NULL;
        mngt->adv_tx[0].next_state = retransmit ? PKT_BUILD_NORMAL : PKT_BUILD_WITH_SEQ_AUTH;
    }
    for(i=1;i<=mngt->SegN;++i)
    {
        if((mngt->block_ack&1<<i)==0)
        {
            mngt->adv_tx[i].adv = NULL;
            mngt->adv_tx[i].next_state = PKT_BUILD_NORMAL;
        }
    }
}

static bool upper_tx_mngt_release(upper_tx_mngt_t *ptr)
{
    bool free = false;
    if(ptr)
    {
        if(ptr->segment_tx_timer)
        {
            mesh_timer_delete(ptr->segment_tx_timer);
        }
        mesh_free(ptr->adv_tx);
        bool released = array_buf_release(&upper_tx_mngt_buf,ptr);
        BX_ASSERT(released);
        free = true;
    }
    return free;
}

static upper_tx_env_t *upper_tx_env_alloc(void)
{
    upper_tx_env_t *env = array_buf_calloc(&upper_tx_env_buf);
    return env;
}

static void upper_tx_env_release(upper_tx_env_t *env)
{
    if(env)
    {
        LOG(LOG_LVL_INFO,"env_release : 0x%x \n",env);
        array_buf_release(&upper_tx_env_buf,env);
    }
    BX_ASSERT(env);
}

static void seg_tx_timer_start(upper_tx_mngt_t *mngt,uint32_t ms)
{
    mesh_timer_change_period(mngt->segment_tx_timer,pdMS_TO_TICKS(ms));
}

static void seg_tx_timer_restart(upper_tx_mngt_t *mngt)
{
    mesh_timer_reset(mngt->segment_tx_timer);
}

static bool seg_tx_timer_cancel(upper_tx_mngt_t *mngt)
{
    return mesh_timer_cancel(mngt->segment_tx_timer);
    }

static void upper_tx_build(upper_tx_mngt_t *mngt,upper_pdu_tx_base_t *pdu_base,uint8_t SegO,bool first_pkt)
{
    void *pdu ;
    lower_pdu_build_fn_t pdu_build_func;
    if(pdu_base->ctl)
    {
        pdu = CONTAINER_OF(pdu_base,control_pdu_tx_t,pdu_base);
    }else
    {
        pdu = CONTAINER_OF(pdu_base,access_pdu_tx_t,pdu_base);
    }
    if(pdu_base->seg)
    {
        pdu_build_func = pdu_base->ctl ? (lower_pdu_build_fn_t)seg_control_pdu_build : (lower_pdu_build_fn_t)seg_access_pdu_build;
    }else
    {
        pdu_build_func = pdu_base->ctl ? (lower_pdu_build_fn_t)unseg_control_pdu_build : (lower_pdu_build_fn_t)unseg_access_pdu_build;
    }
    network_pdu_tx_t *tx = pdu_build_func(pdu,pdu_base->netkey_credentials->nid,SegO, mngt->SegN,first_pkt);
    tx->netkey_credentials = pdu_base->netkey_credentials;
    tx->iv_index = pdu_base->iv_index;
    tx->pkt_type = UPPER_LAYER_ADV_PKT;
    network_tx_process_start(tx);
}

static void upper_tx_complete(upper_pdu_tx_base_t *base,uint8_t status)
{
    void *pdu;
    if(base->ctl)
    {
    
        pdu = CONTAINER_OF(base,control_pdu_tx_t,pdu_base);
        control_tx_complete(pdu, status);
        //TODO ctrl tx complete
    }else
    {
        pdu = CONTAINER_OF(base,access_pdu_tx_t,pdu_base);
        access_tx_complete(pdu,status);
    }

}

upper_pdu_tx_base_t *upper_tx_env_get_current(upper_tx_env_t *env)
{
    struct co_list_hdr *hdr = co_list_pick(&env->tx_list);
    return CONTAINER_OF(hdr, upper_pdu_tx_base_t, hdr);
}

static void set_upper_adv_tx_state_done(upper_adv_tx_env_t *ptr)
{
    ptr->next_state = PKT_DONE;
    ptr->adv = NULL;
}

static void upper_tx_pkt_build_state_process(upper_tx_mngt_t *mngt,upper_pdu_tx_base_t *pdu,uint8_t SegO)
{
    upper_adv_tx_env_t *tx = &mngt->adv_tx[SegO];
    if(tx->next_state == PKT_CANCEL)
    {
        set_upper_adv_tx_state_done(tx);
    }else
    {
        BX_ASSERT(tx->next_state == PKT_BUILD_WITH_SEQ_AUTH || tx->next_state == PKT_BUILD_NORMAL);
        upper_tx_build(mngt,pdu,SegO,tx->next_state == PKT_BUILD_WITH_SEQ_AUTH);
        tx->next_state = PKT_BUILT_COMPLETE;
    }

}

void upper_tx_setup_start(upper_tx_env_t *env)
{
    upper_pdu_tx_base_t *pdu = upper_tx_env_get_current(env);
    upper_tx_mngt_t *mngt = upper_tx_mngt_alloc(pdu,env);
    BX_ASSERT(mngt);
    env->mngt = mngt;
    upper_tx_adv_pkt_state_setup(mngt,false);
    mngt->SegO_to_build = 0;
    upper_tx_pkt_build_state_process(mngt,pdu,mngt->SegO_to_build);
}

static void upper_tx_new_pdu(upper_tx_env_t *env)
{
    upper_pdu_tx_base_t *pdu = upper_tx_env_get_current(env);
    pdu->seq_auth = mesh_seqnum_alloc(1);
    //LOG_D("upper_tx_new_pdu pdu:%x seq_auth:%x\n", pdu, pdu->seq_auth);
    pdu->seq_auth_valid = 1;
    pdu->low_power_mode = low_power_mode_status_get()? 1 : 0;
    if(pdu->ctl)
    {
        upper_tx_setup_start(env);
    }else
    {
        access_tx_process_start(env);
    }
}

void upper_tx_env_add_new_pdu(upper_pdu_tx_base_t *pdu_base)
{
    upper_tx_env_t *env = upper_tx_env_search(pdu_base->dst_addr,pdu_base->ctl,NULL);
    bool empty;
    if(env == NULL)
    {
        env = upper_tx_env_alloc();
        BX_ASSERT(env);
        empty = true;
    }else
    {
        empty = false;
    }
    BX_ASSERT(empty == co_list_is_empty(&env->tx_list));
    co_list_push_back(&env->tx_list,&pdu_base->hdr);
    if(empty)
    {
        upper_tx_new_pdu(env);
    }
}

static void upper_tx_pdu_finish_and_next(upper_tx_env_t *env,uint8_t status)
{
    bool released = upper_tx_mngt_release(env->mngt);
    BX_ASSERT(released);
    env->mngt = NULL;
    upper_pdu_tx_base_t *pdu = upper_tx_env_get_current(env);
    upper_tx_complete(pdu,status);
    co_list_pop_front(&env->tx_list);
    if(co_list_is_empty(&env->tx_list)==false)
    {
        upper_tx_new_pdu(env);
    }else
    {
        upper_tx_env_release(env);
    }
}

static bool segmented_pdu_retransmit_cnt_expire(upper_tx_mngt_t *mngt)
{
    return mngt->tx_cnt == SEGMENTED_PDU_RETRANSMIT_MAX_TIMES_UNICAST;
}

static uint8_t upper_tx_env_get_next_sego(uint32_t block_ack,uint8_t start,uint8_t SegN)
{
    uint8_t sego;
    for(sego = start;(block_ack&1<<sego)!=0&&sego<=SegN;++sego);
    return sego;
}

static void unacked_segment_retransmit(upper_tx_env_t *env)
{
    upper_pdu_tx_base_t *pdu = upper_tx_env_get_current(env);
    upper_tx_mngt_t *mngt = env->mngt;
    upper_tx_adv_pkt_state_setup(mngt,true);
    mngt->need_resched = false;
    ++mngt->tx_cnt;
    mngt->SegO_to_build = upper_tx_env_get_next_sego(mngt->block_ack,0,mngt->SegN);
    upper_tx_pkt_build_state_process(mngt,pdu,mngt->SegO_to_build);
}

static void segment_retransmission_control_unicast(upper_tx_env_t *env)
{
    if(segmented_pdu_retransmit_cnt_expire(env->mngt))
    {
        upper_tx_pdu_finish_and_next(env,UPPER_PDU_TX_CANCELED);
    }else
    {
        seg_tx_timer_restart(env->mngt);
        unacked_segment_retransmit(env);
    }
}

static void upper_tx_reschedule(upper_tx_env_t *env,upper_pdu_tx_base_t *pdu)
{
    upper_tx_mngt_t *mngt = env->mngt;
    if(segment_tx_timer_check(pdu))
    {
            if(mngt->block_ack == (1<< mngt->SegN + 1) - 1)
            {
                upper_tx_pdu_finish_and_next(env,UPPER_PDU_TX_COMPLETE);
            }else if(mngt->block_ack == 0 && mngt->ack_received)
            {
                upper_tx_pdu_finish_and_next(env,UPPER_PDU_TX_CANCELED);
            }else
            {
                segment_retransmission_control_unicast(env);
            }
    }else
    {
//         LOG(LOG_LVL_WARN,"reschedule : 0x%x, %d  \n",pdu,pdu->seg);
        if(pdu->seg == 0)
        {
            upper_tx_pdu_finish_and_next(env,UPPER_PDU_TX_COMPLETE);
        }else
        {
            segment_retransmission_control_virtual_group(env);
        }
    }
}

static bool upper_tx_resched_condition_check(upper_tx_mngt_t *mngt)
{
    if(mngt->need_resched == false)
    {
        return false;
    }
    uint8_t i;
    for(i = 0 ;i<=mngt->SegN; ++i)
    {
        if(mngt->adv_tx[i].next_state != PKT_DONE)
        {
            return false;
        }
    }
    return true;
}

static void upper_tx_reschedule_if_necessary(upper_tx_env_t *env)
{
    if(upper_tx_resched_condition_check(env->mngt))
    {
        upper_pdu_tx_base_t *pdu = upper_tx_env_get_current(env);
        upper_tx_reschedule(env,pdu);
    }
}

static void cancel_ongoing_tx_pkt(upper_tx_env_t *env)
{
    upper_pdu_tx_base_t *pdu = upper_tx_env_get_current(env);
    upper_tx_mngt_t *mngt = env->mngt;
    uint8_t i;
    for(i=0;i<=mngt->SegN;++i)
    {
        upper_adv_tx_env_t *ptr = &mngt->adv_tx[i];
        switch(ptr->next_state)
        {
        case PKT_IDLE:
            BX_ASSERT(0);
        break;
        case PKT_BUILD_WITH_SEQ_AUTH:
        case PKT_BUILD_NORMAL:
            set_upper_adv_tx_state_done(ptr);
        break;
        case PKT_BUILT_COMPLETE:
            ptr->next_state = PKT_CANCEL;
        break;
        case PKT_ADV_START:
        case PKT_ADV_INTERVAL:
            BX_ASSERT(ptr->adv);
            if(network_adv_tx_cancel(ptr->adv))
            {
                set_upper_adv_tx_state_done(ptr);
            }else
            {
                ptr->next_state = PKT_CANCEL;
            }
        break;
        case PKT_CANCEL:
        case PKT_DONE:
        break;
        default:
            BX_ASSERT(0);
        break;
        }
    }
    mngt->need_resched = true;
    upper_tx_reschedule_if_necessary(env);
}

void segment_ack_rx(uint16_t addr,uint16_t seq_zero,uint32_t block_ack)
{
    upper_pdu_tx_base_t *pdu;
    upper_tx_env_t * env = upper_tx_env_search(addr,0,&pdu);
    if(env && env->mngt && IS_UNICAST_ADDR(pdu->dst_addr) && 
        pdu->seq_auth_valid && LSB(pdu->seq_auth,SEQZERO_LENGTH)==seq_zero)
    {
        upper_tx_mngt_t *mngt = env->mngt;
        BX_ASSERT((block_ack& ((1<< mngt->SegN + 1) - 1)) == block_ack);
        mngt->ack_received = true;
        BX_ASSERT((mngt->block_ack & block_ack) == mngt->block_ack);
        mngt->block_ack = block_ack;
        bool cancelled = true;
        if(mngt->segment_tx_timer)
        {
            cancelled = seg_tx_timer_cancel(mngt);
        }
        if(cancelled)
        {
            cancel_ongoing_tx_pkt(env);
        }
    }else
    {
        LOG(LOG_LVL_WARN,"invalid ack\n");
    }
}

static bool segment_tx_cnt_expire_virtual_group(upper_tx_mngt_t *mngt)
{
    return mngt->tx_cnt == SEGMENTED_PDU_RETRANSMIT_MAX_TIMES_VIRTUAL_GROUP;
}

static void segment_retransmission_control_virtual_group(upper_tx_env_t *env)
{
    if(segment_tx_cnt_expire_virtual_group(env->mngt))
    {
        upper_tx_pdu_finish_and_next(env,UPPER_PDU_TX_COMPLETE);
    }else
    {
        unacked_segment_retransmit(env);
    }
}

static uint8_t get_network_adv_index(upper_tx_mngt_t *mngt,network_adv_tx_t *ptr)
{
    uint8_t i;
    for(i=0;i<=mngt->SegN;++i)
    {
        if(mngt->adv_tx[i].adv==ptr)
        {
            return i;
        }
    }
    BX_ASSERT(0);
    return 0xff;
}

bool upper_adv_tx_start_hook(network_adv_tx_t *ptr,bool adv_cnt_expire,uint16_t *interval_ms)
{
    upper_pdu_tx_base_t *pdu;
    upper_tx_env_t *env = upper_tx_env_search(ptr->dst_addr,ptr->ctl,&pdu);
    *interval_ms = pdu->interval_ms;
    upper_tx_mngt_t *mngt = env->mngt;
    uint8_t SegO = get_network_adv_index(mngt,ptr);
    upper_adv_tx_env_t *tx = &mngt->adv_tx[SegO];
    bool force_stop;
    if(tx->next_state == PKT_CANCEL)
    {
        set_upper_adv_tx_state_done(tx);
        force_stop = true;
    }else if(tx->next_state == PKT_ADV_INTERVAL)
    {
        if(adv_cnt_expire)
        {
            set_upper_adv_tx_state_done(tx);
        }else
        {
            tx->next_state = PKT_ADV_START;
        }
        force_stop = false;
        //build next pkt
        if(ptr->param.count == 1)
        {
            upper_adv_tx_build_next_pkt(mngt,pdu);
        }
    }else
    {
        BX_ASSERT(0);
    }
  //  LOG(LOG_LVL_WARN,"start_hook : 0x%x, %d %d %d\n",ptr,mngt->need_resched,mngt->SegN,mngt->SegO_to_build);
    upper_tx_reschedule_if_necessary(env);
    return force_stop;
}

bool upper_tx_env_adv_tx_timer_expire_handler(network_adv_tx_t *ptr)
{
    upper_pdu_tx_base_t *pdu;
    upper_tx_env_t *env = upper_tx_env_search(ptr->dst_addr,ptr->ctl,&pdu);
    upper_tx_mngt_t *mngt = env->mngt;
    uint8_t SegO = get_network_adv_index(mngt,ptr);
    upper_adv_tx_env_t *tx = &mngt->adv_tx[SegO];
    bool adv_continue;
    if(tx->next_state == PKT_ADV_START)
    {
        tx->next_state = PKT_ADV_INTERVAL;
        adv_continue = true;
    }else if(tx->next_state == PKT_CANCEL)
    {
        set_upper_adv_tx_state_done(tx);
        adv_continue = false;
    }else
    {
        BX_ASSERT(0);
    }
    upper_tx_reschedule_if_necessary(env);
    return adv_continue;
}

static bool upper_adv_tx_build_next_pkt(upper_tx_mngt_t *mngt,upper_pdu_tx_base_t *pdu)
{
    uint8_t next_SegO = upper_tx_env_get_next_sego(mngt->block_ack,mngt->SegO_to_build + 1,mngt->SegN);
//    LOG(LOG_LVL_WARN,"next_pkt : 0x%x, %d %d %d \n",pdu,mngt->SegN,mngt->SegO_to_build,next_SegO);
    if(next_SegO<=mngt->SegN)
    {
        mngt->SegO_to_build = next_SegO;
        upper_tx_pkt_build_state_process(mngt,pdu, mngt->SegO_to_build);
        return true;
    }else
    {

        if(segment_tx_timer_check(pdu)==false)
        {
            mngt->need_resched = true;
        }
        return false;
    }
}

static void start_seg_tx_timer_for_seg_pkt_to_unicast_addr(upper_tx_mngt_t *mngt,upper_pdu_tx_base_t *pdu,uint8_t SegO)
{
    if(segment_tx_timer_check(pdu))
    {
        if(mngt->tx_cnt == 0 && SegO == 0)
        {
            uint32_t ms = pdu->low_power_mode ? LOW_POWER_SEG_TX_TIMER_FACTOR * low_power_poll_interval() : SEG_TX_TIMER(pdu->ttl);
            seg_tx_timer_start(mngt,ms);
        }
    }
}

static bool is_upper_pkt_high_priority(upper_pdu_tx_base_t *base)
{
    if(base->ctl)
    {
        control_pdu_tx_t *pdu = CONTAINER_OF(base,control_pdu_tx_t,pdu_base);
        return pdu->high_priority?true:false;
    }else
    {
        return false;
    }
}

uint32_t upper_pkt_tx_start(uint16_t dst_addr,uint8_t ctl,bool *local,bool *adv,uint32_t *gatt_mask,network_adv_tx_t *adv_tx)
{
    upper_pdu_tx_base_t *pdu;
    upper_tx_env_t * env = upper_tx_env_search(dst_addr,ctl,&pdu);
    upper_tx_mngt_t *mngt = env->mngt;
    uint8_t SegO = mngt->SegO_to_build;
    upper_adv_tx_env_t *tx = &mngt->adv_tx[SegO];
    if(tx->next_state == PKT_CANCEL)
    {
        *local = false;
        *adv = false;
        *gatt_mask = 0;
    }else
    {
        BX_ASSERT(tx->next_state == PKT_BUILT_COMPLETE);
        start_seg_tx_timer_for_seg_pkt_to_unicast_addr(mngt,pdu,SegO);
    }
    if(*adv)
    {
        adv_tx->param.repeats = pdu->repeats;
        adv_tx->param.high_priority = is_upper_pkt_high_priority(pdu);
        tx->adv = adv_tx;
        tx->next_state = PKT_ADV_INTERVAL;
    }else
    {
        set_upper_adv_tx_state_done(tx);
    }
    upper_tx_reschedule_if_necessary(env);
    uint32_t delay_ticks = 0;
    if(pdu->expected_tx_time_valid)
    {
        pdu->expected_tx_time_valid = 0;
//        LOG_I("expected_tx_time %d,current %d",pdu->expected_tx_time.time_cnt,ble_current_time_get().time_cnt);
        ble_txrx_time_t time;
        int32_t diff_time = mesh_time_diff(pdu->expected_tx_time,ble_current_time_get());
        if(diff_time < 0)
        {
            delay_ticks = 0;
        }else
        {
            time.time_cnt = diff_time;
            delay_ticks = ble_time_to_os_tick(time);
        }
    }
    return delay_ticks;
}

