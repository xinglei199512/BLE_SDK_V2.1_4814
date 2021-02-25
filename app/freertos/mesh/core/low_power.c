#define LOG_TAG        "low_power.c"
#define LOG_LVL        LVL_DBG
#include "bx_log.h"

#include "low_power.h"
#include "control_tx_process.h"
#include "timer_wrapper.h"
#include "mesh_gatt_cfg.h"
#include "mesh_sched.h"
#include "mesh_queued_msg.h"
#include "co_endian.h"
#include "mesh_core_api.h"
#include "mesh_env.h"
#include "k2_derivation.h"
#include "lower_rx_process.h"
#include "mesh_kr_comm.h"
#include "mesh_kr_server.h"
#include "mesh_iv_operation_ex.h"
#include "stack_mem_cfg.h"
#include "proxy_s.h"
#define FRIEND_REQUEST_PDU_LENGTH 10
#define FRIEND_POLL_PDU_LENGTH 1


static friend_security_t lpn_security;
static uint16_t lpn_counter;
static uint16_t friend_addr = MESH_ADDR_UNASSIGNED;
static mesh_timer_t lpn_poll_timer;
static mesh_timer_t lpn_polltimeout_timer;
static mesh_timer_t lpn_scan_timer;
static uint8_t lpn_fsn;
static enum{
    STOP_IN_ILDE,
    SLEEP_AFTER_REQUEST,
    SCAN_AFTER_REQUEST,
    STOP_AFTER_OFFER,
    SLEEP_AFTER_POLL,
    SCAN_AFTER_POLL,
}lpn_scan_timer_state;
static friend_offer_payload_t friend_offer_info;
static bool friendship_established;
static uint8_t friend_md;
static struct{
    uint32_t poll_timeout;
    friend_req_criteria_t criteria;
    uint8_t receive_delay;
}friend_request_param;
static uint8_t request_attempt_cnt;
static uint16_t lp_netkey_global_idx;

//extern  void send_heartbeat_massage(uint16_t features);

static void lpn_friend_security_credentials_remove(void);

uint32_t get_lp_polltimeout_value(void)
{
    return friend_request_param.poll_timeout;
}

void low_power_toggle_fsn(void)
{
 //   if(friendship_established && src_addr == friend_addr)
    if(friendship_established )
    {
        lpn_fsn = 1 - lpn_fsn;
    }
}

void set_lp_netkey_global_idx(uint16_t global_idx)
{
    lp_netkey_global_idx = global_idx;
}
uint16_t get_default_netkey_global_idx(void)
{
    dm_netkey_handle_t handle[NETWORK_MATCHED_KEY_MAX_NUM];
    uint32_t count=0;
    uint16_t global_idx = 0xFFFF;
    dm_get_all_netkey_handles(handle , &count);
    if(count > 0)
    {
        global_idx = handle[0]->global_idx;
        LOG_D("get_default_netkey_global_idx:global_idx=0x%x,nid=0x%x",global_idx,handle[0]->key[0].master.nid);
    }
    return global_idx;
}
void set_lp_default_netkey_global_idx(void)
{

    lp_netkey_global_idx = get_default_netkey_global_idx();
    LOG_D("lp_netkey_global_idx=0x%x",lp_netkey_global_idx);
}

static void lpn_scan_timer_expire_handler(void *param)
{
    LOG_D("%s : state %d",__func__,lpn_scan_timer_state);
    switch(lpn_scan_timer_state)
    {
    case SLEEP_AFTER_REQUEST:
        mesh_sched_start_scan();
        lpn_scan_timer_state = SCAN_AFTER_REQUEST;
        mesh_timer_change_period(lpn_scan_timer, pdMS_TO_TICKS(1000));
    break;
    case SCAN_AFTER_REQUEST:
        mesh_sched_stop_scan(NULL);
        if(request_attempt_cnt==FRIEND_REQUEST_ATTEMPTS_MAX)
        {
            //TODO
        }else
        {
            friend_request_tx();
            request_attempt_cnt +=1;
        }
    break;
    case SLEEP_AFTER_POLL:
        {
        mesh_sched_start_scan();
        lpn_scan_timer_state = SCAN_AFTER_POLL;
        uint32_t ticks = pdMS_TO_TICKS(friend_offer_info.receive_window);
        mesh_timer_change_period(lpn_scan_timer, ticks);
        }
    break;
    case SCAN_AFTER_POLL:
        mesh_sched_stop_scan(NULL);
        friend_poll_tx();
    break;
    }
}

static void lpn_scan_timer_callback(mesh_timer_t timer)
{
    mesh_queued_msg_send(lpn_scan_timer_expire_handler, NULL);
}

void low_power_init(void)
{
    lpn_scan_timer = mesh_timer_create("lpn",1,pdFALSE,NULL, lpn_scan_timer_callback);
    
}

void friend_request_tx_done_callback(void *pdu,uint8_t status)
{
    lpn_scan_timer_state = SLEEP_AFTER_REQUEST;
    mesh_timer_change_period(lpn_scan_timer,pdMS_TO_TICKS(100));
}

void friend_request_param_set(uint32_t poll_timeout,friend_req_criteria_t criteria,uint8_t receive_delay)
{
    friend_request_param.poll_timeout = poll_timeout;
    friend_request_param.criteria = criteria;
    friend_request_param.receive_delay = receive_delay;
}


security_credentials_t *lpn_tx_net_security_get(void)
{
    mesh_key_refresh_phase_t phase = MESH_KEY_REFRESH_PHASE_INVALID;
    dm_netkey_kr_phase_get(lpn_security.netkey,&phase);
    if(phase != MESH_KEY_REFRESH_PHASE_2)
    {
        return &lpn_security.credentials[dm_netkey_get_primary_index(lpn_security.netkey)];
    }
    return &lpn_security.credentials[dm_netkey_get_update_index(lpn_security.netkey)];
}

bool lpn_is_friend_security(security_credentials_t * p_security_credentials)
{
    if(p_security_credentials == &lpn_security.credentials[dm_netkey_get_primary_index(lpn_security.netkey)] 
       || p_security_credentials == &lpn_security.credentials[dm_netkey_get_update_index(lpn_security.netkey)])
    {
        return true;
    }else{
        return false;
    }
}

void friend_request_tx(void)
{
    LOG_I("%s",__func__);
    security_credentials_t *net_security;//TODO-FRIEND

    dm_netkey_get_netkey_tx_credentials_by_idx(lp_netkey_global_idx,&net_security);
    
    control_msg_tx_param_t param = {
        .expected_tx_time = NULL,
        .dst_addr = MESH_ALL_FRIENDS_ADDR,
        .length = FRIEND_REQUEST_PDU_LENGTH,
        .netkey_credentials = net_security,
        .opcode = FRIEND_REQUEST,
        .ttl= 0,
        .high_priority = false,
    };
    control_pdu_tx_t *pdu = control_unseg_msg_build(&param,friend_request_tx_done_callback);
    BX_ASSERT(pdu);
    friend_request_payload_t *ptr = (void*)pdu->payload;
    ptr->criteria = friend_request_param.criteria;
    ptr->receive_delay = friend_request_param.receive_delay;
    ptr->poll_timeout_be = THREE_BYTES_ENDIAN_REVERSE(friend_request_param.poll_timeout);
    ptr->prev_addr_be = co_bswap16(friend_addr);
    ptr->elem_nums = get_element_num();
    ptr->lpn_counter_be = co_bswap16(++lpn_counter);//make sure lpn_counter is the same as k2_derivation value.
    control_send(pdu);
}


static void lpn_poll_timer_cleanup()
{
    if(lpn_poll_timer)
    {
        mesh_timer_stop(lpn_poll_timer);
        mesh_timer_delete(lpn_poll_timer);
        lpn_poll_timer = NULL;
    }
}

static void lpn_friendship_termiate()
{
    lpn_scan_timer_state = STOP_IN_ILDE;
    mesh_timer_cancel(lpn_scan_timer);
    lpn_poll_timer_cleanup();
    lpn_friend_security_credentials_remove();
    if(friendship_established)
    {
        friendship_established = false;
        lower_rx_env_reassembly_cleanup();
//        send_heartbeat_massage(0);

    }
}


static void lpn_friend_clear_tx_done_callback(void *pdu,uint8_t status)
{
    lpn_friendship_termiate();
}

void lpn_friend_clear_tx(void)
{
    if(friendship_established == true)
    {
        LOG_I("%s",__func__);
        security_credentials_t *net_security;
        dm_netkey_get_netkey_tx_credentials(lpn_security.netkey,&net_security);
        friend_clear_tx(friend_addr,0,net_security,mesh_node_get_primary_element_addr(),lpn_counter,lpn_friend_clear_tx_done_callback);
    }
}

static void friend_poll_tx_done_callback(void *pdu,uint8_t status)
{
    lpn_scan_timer_state = SLEEP_AFTER_POLL;
    uint32_t ticks = pdMS_TO_TICKS(friend_request_param.receive_delay);
    BX_ASSERT(ticks);
//    LOG_I("delay ticks %d\n",ticks);
    mesh_timer_change_period(lpn_scan_timer,ticks);
}

void friend_poll_tx(void)
{
    LOG_I("%s fsn %d",__func__,lpn_fsn);
    security_credentials_t *net_security;
    net_security = lpn_tx_net_security_get();
    
    control_msg_tx_param_t param=
    {
        .expected_tx_time = NULL,
        .netkey_credentials = net_security,
        .dst_addr = friend_addr,
        .length = FRIEND_POLL_PDU_LENGTH,
        .opcode = FRIEND_POLL,
        .ttl = 0,
        .high_priority = false,
    };
    control_pdu_tx_t *ptr = control_unseg_msg_build(&param, friend_poll_tx_done_callback);
    BX_ASSERT(ptr);
    friend_poll_payload_t *pdu = (friend_poll_payload_t *)ptr->payload;
    pdu->fsn = lpn_fsn;
    pdu->padding = 0;
    control_send(ptr);
//    extern  void io_pin_clear(uint8_t pin_num);
//    io_pin_clear(3);
}

static void lpn_poll_timer_expire_handler(void *param)
{
    LOG_D("%s",__func__);
    if(friendship_established)
    {
        friend_poll_tx();
    }
}

static void lpn_poll_timer_callback(mesh_timer_t timer)
{
    mesh_queued_msg_send(lpn_poll_timer_expire_handler,NULL);
}

static void lpn_friend_security_credentials_remove()
{
    bool remove = co_list_extract(&lpn_security.netkey->friend, &lpn_security.hdr, 0);
    BX_ASSERT(remove);
}

static void lpn_polltimeout_timer_expire_handler(void *param)
{
    lpn_friendship_termiate();
}

static void lpn_polltimeout_timer_callback(mesh_timer_t timer)
{
    mesh_queued_msg_send(lpn_polltimeout_timer_expire_handler,NULL);
}

uint32_t low_power_poll_interval(void)
{
    return friend_request_param.poll_timeout*100/2;
}

static void friend_key_refresh_handle(net_key_t
 * p_netkey,bool key_refresh)
{
     if(key_refresh)
     {
         mesh_kr_netkey_action(p_netkey,MESH_KEY_TRANSITION_TO_PHASE_2);
     }
     else
     {
         mesh_kr_netkey_action(p_netkey,MESH_KEY_TRANSITION_TO_PHASE_3);
     }
}

static void friend_iv_index_handle(uint32_t iv_index, bool iv_update)
{
     mesh_iv_sec_bean_rx(NULL, iv_index, iv_update, false);
}

void friend_update_rx_handler(upper_pdu_rx_t *ptr)
{
    LOG_I("%s",__func__);
    friend_update_payload_t *pdu = (friend_update_payload_t *)ptr->src;
    if(friendship_established == false)
    {
        friendship_established = true;
        lpn_poll_timer = mesh_timer_create("lpn poll",pdMS_TO_TICKS(low_power_poll_interval()),pdTRUE, NULL, lpn_poll_timer_callback);
        mesh_timer_start(lpn_poll_timer);
        proxy_service_beacon_stop();
//        send_heartbeat_massage(8<<8);
    }
    friend_key_refresh_handle(ptr->netkey,pdu->flags.iv_update_active);
    friend_iv_index_handle(co_bswap32(pdu->iv_index_be),pdu->flags.iv_update_active);
    friend_md = pdu->md;

}

static void lpn_k2_derivation_complete(k2_derivation_buf_t *param,void *dummy,uint8_t status)
{
    LOG_D("%s",__func__);
    lpn_fsn = 0;
    friend_poll_tx();
    lpn_polltimeout_timer = mesh_timer_create("lpn polltimeout",pdMS_TO_TICKS(friend_request_param.poll_timeout*100), pdFALSE, NULL, lpn_polltimeout_timer_callback);
    mesh_timer_start(lpn_polltimeout_timer);
}

void friend_offer_judge(upper_pdu_rx_t *ptr)
{
    LOG_I("%s",__func__);
    friend_offer_payload_t *pdu = (friend_offer_payload_t *)ptr->src;

    friend_offer_info = *pdu;
    friend_addr = ptr->src_addr;
    lpn_security.netkey = ptr->netkey;
    co_list_push_back(&ptr->netkey->friend,&lpn_security.hdr);
    net_key_box_t *netkey_box = CONTAINER_OF(ptr->net_security, net_key_box_t, master);
    k2_derivation_buf_t k2_param = {
        .n = netkey_box->netkey,
        .friend = {
            .friend_cnt = co_bswap16(pdu->friend_counter_be),
            .lpn_cnt = lpn_counter,
            .friend_addr = friend_addr,
            .lpn_addr = mesh_node_get_primary_element_addr(),
        },
        .rslt = &lpn_security.credentials[&ptr->netkey->key[0]==netkey_box?0:1],
        .master = false,
    };
    k2_derivation_start(&k2_param, lpn_k2_derivation_complete);


    mesh_sched_stop_scan(NULL);
}

void friend_offer_rx_handler(upper_pdu_rx_t *ptr)
{
    LOG_I("%s",__func__);
    if(lpn_scan_timer_state==SCAN_AFTER_REQUEST)
    {
        if(mesh_timer_cancel(lpn_scan_timer))
        {
            lpn_scan_timer_state = STOP_AFTER_OFFER;
            friend_offer_judge(ptr);
        }
    }else if(lpn_scan_timer_state == STOP_AFTER_OFFER)
    {
        friend_offer_judge(ptr);
    }else
    {
        //LOG
    }

}

static void friend_subscription_list_add_tx_done_callback(void *pdu,uint8_t status)
{
    LOG_I("%s",__func__);
}

void friend_subscription_list_add_tx(uint8_t transaction_number,uint16_t *address_list , uint8_t count)
{
    LOG_I("%s",__func__);
    LOG_I("transaction_tx=0x%x\n",transaction_number);
    security_credentials_t *net_security = lpn_tx_net_security_get();
    
    control_msg_tx_param_t param=
    {
        .expected_tx_time = NULL,
        .netkey_credentials = net_security,
        .dst_addr = friend_addr,
        .length = 1+2*count,
        .opcode = FRIEND_SUBSCRIPTION_LIST_ADD,
        .ttl = 0,
        .high_priority = false,
    };
    control_pdu_tx_t *ptr = control_unseg_msg_build(&param, friend_subscription_list_add_tx_done_callback);
    BX_ASSERT(ptr);
    friend_sunscription_list_add_payload_t *pdu = (void *)ptr->payload;
    pdu->transaction_number = transaction_number;
    for(uint8_t i=0;i<count;i++)
    {
        pdu->address_list_be[i] = co_bswap16(address_list[i]);
    }
    control_send(ptr);
}

static void friend_subscription_list_remove_tx_done_callback(void *pdu,uint8_t status)
{
    LOG_I("%s",__func__);
}

void friend_subscription_list_remove_tx(uint8_t transaction_number,uint16_t *address_list , uint8_t count)
{
    LOG_I("%s",__func__);
    LOG_I("transaction_tx=0x%x\n",transaction_number);
    security_credentials_t *net_security = lpn_tx_net_security_get();
    
    control_msg_tx_param_t param=
    {
        .expected_tx_time = NULL,
        .netkey_credentials = net_security,
        .dst_addr = friend_addr,
        .length = 1+2*count,
        .opcode = FRIEND_SUBSCRIPTION_LIST_REMOVE,
        .ttl = 0,
        .high_priority = false,
    };
    control_pdu_tx_t *ptr = control_unseg_msg_build(&param, friend_subscription_list_remove_tx_done_callback);
    BX_ASSERT(ptr);
    friend_sunscription_list_remove_payload_t *pdu = (void *)ptr->payload;
    pdu->transaction_number = transaction_number;
    for(uint8_t i=0;i<count;i++)
    {
        pdu->address_list_be[i] = co_bswap16(address_list[i]);
    }
    control_send(ptr);

}

void friend_subscription_list_confirm_rx_handler(upper_pdu_rx_t *ptr)
{
    LOG_I("%s",__func__);
    friend_sunscription_list_confirm_payload_t *pdu=(void *)ptr->src;
    LOG_I("transaction_rx=0x%x\n",pdu->transaction_number);
}

void lpn_schedule_at_pkt_rx(bool is_update_pkt)
{
    LOG_I("%s",__func__);
    if(mesh_timer_active_then_reset(lpn_polltimeout_timer))
    {
        LOG_D("lpn_polltimeout_timer");
        if(mesh_timer_cancel(lpn_scan_timer))
        {
            LOG_D("lpn_scan_timer");
            mesh_sched_stop_scan(NULL);
            if((is_update_pkt && friend_md) || (!is_update_pkt))
            {
                LOG_D("friend_md");
                friend_poll_tx();
            }
        }
    }
}

bool low_power_mode_status_get(void)
{
    return friendship_established;
}

uint16_t  low_power_friend_addr(void)
{
    return friend_addr;
}



err_t low_power_get_friend_tx_credentials_by_idx(mesh_global_idx_t netkey_idx, security_credentials_t ** pp_security_credentials)
{
     if(!friendship_established)
     {
        return MESH_CORE_ERROR_NOT_FOUND;
     }
     if(lpn_security.netkey->global_idx != netkey_idx)
     {
         return MESH_CORE_ERROR_NOT_FOUND;
     }
     if(lpn_security.netkey->key_refresh_phase == MESH_KEY_REFRESH_PHASE_2)
     {
         *pp_security_credentials = &lpn_security.credentials[1 - lpn_security.netkey->primary_used];
     }else
     {
         *pp_security_credentials = &lpn_security.credentials[lpn_security.netkey->primary_used];
     }
     return MESH_CORE_SUCCESS;
}

void low_power_generate_credentials(void* dm_handle)
{
    LOG_I("%s",__func__);
    if(!friendship_established)
    {
        return;
    }
    net_key_box_t *netkey_box = &lpn_security.netkey->key[1 - lpn_security.netkey->primary_used];
    k2_derivation_buf_t k2_param = {
    .n = netkey_box->netkey,
    .friend = {
        .friend_cnt = co_bswap16(friend_offer_info.friend_counter_be),
        .lpn_cnt = lpn_counter,
        .friend_addr = friend_addr,
        .lpn_addr = mesh_node_get_primary_element_addr(),
    },
    .rslt = &lpn_security.credentials[1 - lpn_security.netkey->primary_used],
    .master = false,
    };
    k2_derivation_start(&k2_param, lpn_k2_derivation_complete);

}


