#define LOG_TAG        "friend.c"
#define LOG_LVL        LVL_DBG
#include "bx_log.h"

#include "friendship.h"
#include "friend.h"
#include "co_endian.h"
#include "lower_rx_process.h"
#include "lower_tx_process.h"
#include "upper_pdu.h"
#include "network_keys_dm.h"
#include "string.h"
#include "mesh_iv_operation_ex.h"
#include "osapp_utils.h"
#include "network_tx_process.h"
#include "mesh_env.h"
#include "mesh_queued_msg.h"
#include "timer_wrapper.h"
#include "control_tx_process.h"
#include "security.h"
#include "k2_derivation.h"
#include "task_init.h"





#define FRIEND_RECEIVE_WINDOW   20
#define FRIEND_OFFER_PDU_LENGTH 6
#define FRIEND_CLEAR_CONFIRM_PDU_LENGTH 4
#define FRIEND_UPDATE_PDU_LENGTH 6
#define FRIEND_SUBSCRIPTION_LIST_CONFIRM_LENGTH     1
//Mesh Spec v1.0.1 table3.27 PollTimeout field values
#define FRIEND_POLLTIMEOUT_LOWER_BOUND   (0x00000009)
#define FRIEND_POLLTIMEOUT_UPPER_BOUND   (0x0034BC00)

DEF_ARRAY_BUF(friend_list,friend_list_t,SUPPORTED_LOW_POWER_NODE_NUMS);
static uint16_t friend_counter;




void friend_clear_tx_by_friend(friend_list_t *friend);
static void clear_repeat_timer_cancel_and_delete(friend_list_t *ptr);
static void clear_procedure_timer_cancel_and_delete(friend_list_t *ptr);




uint16_t friend_q_available_size(friend_q_t *ptr)
{
    return array_buf_available_size(GET_ARRAY_BUF_TYPE_HDL(ptr));
}

friend_q_element_t *friend_q_element_alloc(friend_q_t *ptr)
{
    return array_buf_alloc(GET_ARRAY_BUF_TYPE_HDL(ptr));
}

void friend_q_element_release(friend_q_t *ptr,friend_q_element_t *element)
{
    array_buf_release(GET_ARRAY_BUF_TYPE_HDL(ptr),element);
}

void friend_q_element_env_free(friend_q_t *ptr,friend_q_element_t *element)
{
    tx_pdu_buf_release(element->tx_pdu);
    friend_q_element_release(ptr,element);
}

static bool seg_ack_compare(network_pdu_packet_u *ptr,uint16_t src_addr,uint16_t dst_addr,uint16_t seq_zero)
{
    if(ptr->pkt.ctl != 1)
    {
        return false;
    }
    uint8_t *lower_pdu = &ptr->buf[ENCRYPTED_DATA_OFFSET + 2];
    lower_pkt_head_u head = *(lower_pkt_head_u *)lower_pdu;
    if(head.control.opcode != 0)
    {
        return false;
    }
    if(src_addr != co_bswap16(ptr->pkt.src_be))
    {
        return false;
    }
    if(dst_addr != co_bswap16(ptr->pkt.dst_be))
    {
        return false;
    }
    if(seq_zero != lower_pdu_seq_zero_get(lower_pdu))
    {
        return false;
    }
    return true;
}

friend_q_element_t *friend_q_seg_ack_search(friend_list_t *ptr,uint16_t src_addr,uint16_t dst_addr,uint16_t seq_zero)
{
    struct co_list_hdr *hdr = co_list_pick(&ptr->msg_list);
    friend_q_element_t *element = NULL;
    while(hdr)
    {
        element = CONTAINER_OF(hdr,friend_q_element_t,hdr);
        if(seg_ack_compare((network_pdu_packet_u *)&element->tx_pdu->src.head,src_addr,dst_addr,seq_zero))
        {
            return element;
        }
        hdr = co_list_next(hdr);
    }
    return NULL;
}

bool friend_q_remove(friend_list_t *ptr,friend_q_element_t *element)
{
    return co_list_extract(&ptr->msg_list,&element->hdr,0);
}

static bool is_friend_update_msg(network_pdu_packet_u *ptr)
{
    if(ptr->pkt.ctl != 1)
    {
        return false;
    }    
    uint8_t *lower_pdu = &ptr->buf[ENCRYPTED_DATA_OFFSET + 2];
    lower_pkt_head_u head = *(lower_pkt_head_u *)lower_pdu;
    if(head.control.opcode != FRIEND_UPDATE)
    {
        return false;
    }
    return true;
}

friend_q_element_t *friend_q_remove_oldest_entry(friend_list_t *ptr)
{
    struct co_list_hdr *hdr = co_list_pick(&ptr->msg_list);
    friend_q_element_t *element = NULL;
    while(hdr)
    {
        element = CONTAINER_OF(hdr,friend_q_element_t,hdr);  
        if(is_friend_update_msg((network_pdu_packet_u *)&element->tx_pdu->src.head))
        {
            hdr = co_list_next(hdr);
        }else
        {
            break;
        }
    }
    BX_ASSERT(element);
    bool remove = co_list_extract(&ptr->msg_list,hdr,0);
    BX_ASSERT(remove);
    return element;
}

friend_list_t *friend_env_calloc(void)
{
    return array_buf_calloc(&friend_list);
}

void friend_env_retain(friend_list_t *env)
{
    array_buf_retain(&friend_list,env);
}

void friend_env_release(friend_list_t *env)
{
    array_buf_release(&friend_list,env);
}

friend_list_t *friend_search_for_caching_msg(uint16_t dst_addr)
{
    friend_list_t *ptr;
    FOR_EACH_ALLOCATED_ITEM(friend_list, ptr, 
        if(ptr->uni_addr<=dst_addr && ptr->uni_addr + ptr->element_nums > dst_addr)
        {
            return ptr;
        }
        uint16_t i;
        for(i=0;i<SUBSCRIPTION_LIST_SIZE_PER_FRIEND;++i)
        {
            if(ptr->subscript_list[i]==dst_addr)
            {
                return ptr;
            }
        }
    )
    return NULL;
}

friend_list_t *friend_primary_addr_search(uint16_t primary_addr)
{
    friend_list_t *ptr;
    FOR_EACH_ALLOCATED_ITEM(friend_list, ptr, 
        if(ptr->uni_addr == primary_addr)
        {
            return ptr;
        }
    )
    return NULL;
}

static security_credentials_t *friend_tx_net_security_get(friend_list_t *ptr)
{
    mesh_key_refresh_phase_t phase = MESH_KEY_REFRESH_PHASE_INVALID;
    friend_security_t * p_security = &ptr->security;
    dm_netkey_kr_phase_get(p_security->netkey,&phase);
    if(phase != MESH_KEY_REFRESH_PHASE_2)
    {
        return &p_security->credentials[dm_netkey_get_primary_index(p_security->netkey)];
    }
    return &p_security->credentials[dm_netkey_get_update_index(p_security->netkey)];
    
}

void alloc_friend_q_element_and_push_to_list(friend_list_t *env,network_pdu_packet_u *data,uint8_t total_length,struct co_list *list)
{
    if(friend_q_available_size(&env->queue)==0)
    {
        if(env->current_tx)
        {
            friend_q_element_t *old = friend_q_remove_oldest_entry(env);
            friend_q_element_env_free(&env->queue, old);
        }
    }
    network_pdu_tx_t *tx_pdu = tx_pdu_buf_alloc();
    BX_ASSERT(tx_pdu);
    tx_pdu->src.lower_pdu_length = network_transport_pdu_length(total_length,data->pkt.ctl);
    memcpy(&tx_pdu->src.head,data->buf,tx_pdu->src.lower_pdu_length + sizeof(network_pdu_packet_head_t));
    tx_pdu->iv_index = mesh_tx_iv_index_get();
    tx_pdu->netkey_credentials = friend_tx_net_security_get(env);
    tx_pdu->pkt_type = FRIEND_QUEUE_ADV_PKT;
    tx_pdu->src.head.ttl -= 1;
    friend_q_element_t *q_elem = friend_q_element_alloc(&env->queue);
    BX_ASSERT(q_elem);
    q_elem->tx_pdu = tx_pdu;
    co_list_push_back(list,&q_elem->hdr);
}

void friend_q_pop_and_send(friend_list_t *ptr,ble_txrx_time_t expected_tx_time)
{
    LOG_I("%s",__func__);
    if(friend_get_cached_msg_num(ptr)==0)
    {
        mesh_key_refresh_phase_t key_refresh_phase;
        dm_netkey_kr_phase_get(ptr->security.netkey,&key_refresh_phase);
        bool iv_index_update = mesh_iv_update_is_processing();
        friend_update_add_to_q(ptr,key_refresh_phase == MESH_KEY_REFRESH_PHASE_2,iv_index_update);
    }
    struct co_list_hdr *hdr = co_list_pop_front(&ptr->msg_list);
    BX_ASSERT(hdr);
    ptr->current_tx = CONTAINER_OF(hdr, friend_q_element_t, hdr);
    //ptr->current_tx->tx_adv = NULL;
    ptr->expected_tx_time = expected_tx_time;
    if(is_friend_update_msg((network_pdu_packet_u*)&ptr->current_tx->tx_pdu->src.head))
    {
        friend_update_payload_t *payload = (void*)&ptr->current_tx->tx_pdu->src.lower_pdu[1];
        payload->md = friend_get_cached_msg_num(ptr)?1:0;
        LOG_I("!!change md!!  %d\n",payload->md);
    }
    network_tx_process_start(ptr->current_tx->tx_pdu);
}

uint32_t friend_tx_delay_calc(ble_txrx_time_t expected_tx_time)
{
    ble_txrx_time_t time;
    time.time_cnt = mesh_time_diff(expected_tx_time,ble_current_time_get());
    return ble_time_to_os_tick(time);
}

uint32_t friend_queue_pkt_tx_start(uint16_t dst_addr,bool *local,bool *adv,uint32_t *gatt_mask,network_adv_tx_t *adv_tx)
{
    friend_list_t *ptr = friend_search_for_caching_msg(dst_addr);
    adv_tx->param.high_priority = true;
    adv_tx->param.repeats = 0;
    ptr->current_tx->tx_adv = adv_tx;
    network_adv_tx_buf_retain(adv_tx);
    return friend_tx_delay_calc(ptr->expected_tx_time);
}

uint16_t friend_get_cached_msg_num(friend_list_t *env)
{
    return co_list_size(&env->msg_list);
}

void friend_update_add_to_q(friend_list_t *ptr,bool key_refresh_phase_2,bool iv_update_active)
{
    LOG_I("%s",__func__);
    network_pdu_packet_u pdu;
    uint32_t seq = mesh_seqnum_alloc(1);
    pdu.pkt.ctl = 1;
    pdu.pkt.ttl = 1;
    pdu.pkt.seq_be = SEQ_ENDIAN_REVERSE(seq);
    pdu.pkt.src_be = co_bswap16(mesh_node_get_primary_element_addr());
    pdu.pkt.dst_be = co_bswap16(ptr->uni_addr);
    pdu.buf[sizeof(network_pdu_packet_head_t)] = FRIEND_UPDATE;
    friend_update_payload_t *payload = (void*)&pdu.buf[sizeof(network_pdu_packet_head_t)+1];
    payload->flags.key_refresh_phase_2 = key_refresh_phase_2;
    payload->flags.iv_update_active = iv_update_active;
    payload->flags.reserved         = 0;
    payload->iv_index_be = co_bswap32(mesh_beacon_iv_index_get());
    payload->md = friend_get_cached_msg_num(ptr)?1:0;
    alloc_friend_q_element_and_push_to_list(ptr,&pdu,
        FRIEND_UPDATE_PDU_LENGTH+1+sizeof(network_pdu_packet_head_t)+TRANSMIC_64BIT,&ptr->msg_list);
}

static void friend_release_prev_tx_q_element(friend_list_t *ptr)
{
    network_adv_tx_buf_release(ptr->current_tx->tx_adv);
    //BX_ASSERT(released);
    friend_q_element_release(&ptr->queue, ptr->current_tx);
    ptr->current_tx = NULL;

}

static void friend_clear_repeat_timer_expire_handler(void *data)
{
    friend_list_t *ptr = data;
    if(ptr->clear_procedure_timer)
    {
        uint32_t ticks = mesh_timer_get_period(ptr->clear_repeat_timer);
        friend_clear_tx_by_friend(ptr);
        mesh_timer_change_period(ptr->clear_repeat_timer, 2*ticks);
    }else
    {
        clear_repeat_timer_cancel_and_delete(ptr);
    }
}

static void friend_clear_repeat_timer_callback(mesh_timer_t timer)
{
    friend_list_t *data = mesh_timer_get_associated_data(timer);
    mesh_queued_msg_send(friend_clear_repeat_timer_expire_handler,data);
}

static void friend_clear_procedure_timer_expire_handler(void *data)
{
    friend_list_t *ptr = data;
    clear_repeat_timer_cancel_and_delete(ptr);
    clear_procedure_timer_cancel_and_delete(ptr);
}

static void friend_clear_procedure_timer_callback(mesh_timer_t timer)
{
    friend_list_t *data = mesh_timer_get_associated_data(timer);
    mesh_queued_msg_send(friend_clear_procedure_timer_expire_handler,data);
}

bool param_check_in_friend_poll_rx_handler(upper_pdu_rx_t *ptr)
{
    friend_poll_payload_t *payload = (friend_poll_payload_t *)ptr->src;
    if(payload->padding != 0x00)
    {
        LOG_I("[PTS %s]: error padding\n", __func__);
        return 0;
    }
    return 1;
}

void friend_poll_rx_handler(upper_pdu_rx_t *ptr)
{
    LOG_I("%s",__func__);
    friend_list_t *friend = friend_primary_addr_search(ptr->src_addr);
    friend_poll_payload_t *payload = (friend_poll_payload_t *)ptr->src;
    ble_txrx_time_t expected_tx_time = mesh_time_add(ptr->rx_time,mesh_ms_to_ble_time(friend->receive_delay+FRIEND_RECEIVE_WINDOW/2));
    LOG_I("delay ticks %d from %d\n",ble_time_to_os_tick(mesh_ms_to_ble_time(friend->receive_delay+FRIEND_RECEIVE_WINDOW/2)),friend->uni_addr);

    if(param_check_in_friend_poll_rx_handler(ptr) == 0)
    {
        return;
    }

    if(friend->friendship_established)
    {
        if(mesh_timer_active_then_reset(friend->poll_timer))
        {
            BX_ASSERT(friend->current_tx);
            if(payload->fsn == friend->fsn)
            {
//                LOG_D("poll 0x%x\n",friend->current_tx->tx_adv);
//                BX_ASSERT(friend->current_tx->tx_adv);
                network_adv_tx_buf_retain(friend->current_tx->tx_adv);
                network_adv_retransmit_for_friend_q_element(friend->current_tx->tx_adv,friend_tx_delay_calc(expected_tx_time));
                LOG_D("poll retry 0x%x ticks : %d fsn %d\n",friend->current_tx->tx_adv,friend_tx_delay_calc(expected_tx_time),friend->fsn);
            }else
            {
//                LOG_D("new 0x%x\n",friend->current_tx->tx_adv);
                
                friend_release_prev_tx_q_element(friend);
                friend->fsn = payload->fsn;
                friend_q_pop_and_send(friend,expected_tx_time);
                LOG_D("poll new 0x%x ticks : %d fsn %d\n",friend->current_tx->tx_adv,friend_tx_delay_calc(expected_tx_time),friend->fsn);
            }
        }
    }else
    {
        if(mesh_timer_cancel(friend->poll_timer))
        {
            friend->friendship_established = true;
            mesh_timer_change_period(friend->poll_timer,pdMS_TO_TICKS(friend->poll_timeout*100));
            friend->fsn = payload->fsn;
            friend_q_pop_and_send(friend,expected_tx_time);
//            LOG_D("new2 0x%x\n",friend->current_tx->tx_adv);

            if(friend->prev_friend && friend->prev_friend !=mesh_node_get_primary_element_addr())
            {
                friend->clear_repeat_timer = mesh_timer_create("clear repeat", pdMS_TO_TICKS(1000), pdFALSE, 
                    friend, friend_clear_repeat_timer_callback);
                friend->clear_procedure_timer = mesh_timer_create("clear procedure", pdMS_TO_TICKS(friend->poll_timeout*100*2), pdFALSE, 
                    friend, friend_clear_procedure_timer_callback);
                friend_clear_tx_by_friend(friend);
                mesh_timer_start(friend->clear_repeat_timer);
                mesh_timer_start(friend->clear_procedure_timer);
            }
        }
    }
}

static void friend_offer_tx_done_callback(void *pdu,uint8_t status)
{
    control_pdu_tx_t *ptr = pdu;
    friend_list_t *friend = friend_primary_addr_search(ptr->pdu_base.dst_addr);
    mesh_timer_start(friend->poll_timer);
}

void friend_offer_tx(uint16_t dst_addr,uint8_t rssi,ble_txrx_time_t expected_tx_time,security_credentials_t *net_security)
{
    LOG_I("%s",__func__);
    control_msg_tx_param_t param = {
        .expected_tx_time = &expected_tx_time,
        .dst_addr = dst_addr,
        .length = FRIEND_OFFER_PDU_LENGTH,
        .netkey_credentials = net_security,
        .opcode = FRIEND_OFFER,
        .ttl= 0,
        .high_priority = true,
    };
    control_pdu_tx_t *pdu = control_unseg_msg_build(&param,friend_offer_tx_done_callback);
    BX_ASSERT(pdu);
    friend_offer_payload_t *ptr = (void*)pdu->payload;
    ptr->receive_window = FRIEND_RECEIVE_WINDOW;
    ptr->queue_size = QUEUE_LENGTH_PER_FRIEND;
    ptr->subscription_list_size = SUBSCRIPTION_LIST_SIZE_PER_FRIEND;
    ptr->rssi = rssi;
    ptr->friend_counter_be = co_bswap16(++friend_counter);//make sure friend_counter is the same as k2_derivation value.
    control_send(pdu);
}

static void friend_friend_clear_tx_done_callback(void *pdu,uint8_t status)
{

}

void friend_clear_tx_by_friend(friend_list_t *friend)
{
    security_credentials_t *net_security;//TODO-FRIEND
    dm_netkey_get_netkey_tx_credentials(friend->security.netkey,&net_security);
    friend_clear_tx(friend->prev_friend,0x7f,net_security,friend->uni_addr,friend->lpn_counter,friend_friend_clear_tx_done_callback);
}

void friend_queue_list_cleanup(friend_list_t *env,struct co_list *list)
{
    while(1)
    {
        struct co_list_hdr *hdr = co_list_pop_front(list);
        if(hdr==NULL)
        {
            break;
        }
        friend_q_element_t *q_element = CONTAINER_OF(hdr, friend_q_element_t , hdr);
        friend_q_element_env_free(&env->queue, q_element);
    }
}

static void clear_repeat_timer_cancel_and_delete(friend_list_t *ptr)
{
    if(ptr->clear_repeat_timer)
    {
        if(mesh_timer_cancel(ptr->clear_repeat_timer))
        {
            mesh_timer_delete(ptr->clear_repeat_timer);
            ptr->clear_repeat_timer = NULL;
        }
    }
}

static void clear_procedure_timer_cancel_and_delete(friend_list_t *ptr)
{
    if(ptr->clear_procedure_timer)
    {
        if(mesh_timer_cancel(ptr->clear_procedure_timer))
        {
            mesh_timer_delete(ptr->clear_procedure_timer);
            ptr->clear_procedure_timer = NULL;
        }
    }
}

static void friendship_cleanup(friend_list_t *ptr)
{
     LOG_I("%s",__func__);
    if(ptr->friendship_established)
    {
        friend_release_prev_tx_q_element(ptr);
        friend_queue_list_cleanup(ptr,&ptr->msg_list);
        clear_repeat_timer_cancel_and_delete(ptr);
        clear_procedure_timer_cancel_and_delete(ptr);
    }
    mesh_timer_delete(ptr->poll_timer);
    ptr->poll_timer = NULL;
    bool remove = co_list_extract(&ptr->security.netkey->friend, &ptr->security.hdr, 0);
    BX_ASSERT(remove);
    friend_env_release(ptr);
}

static void poll_timer_expire_handler(void *ptr)
{
    friendship_cleanup(ptr);
}

static void poll_timer_callback(mesh_timer_t timer)
{
    friend_list_t *data = mesh_timer_get_associated_data(timer);
    mesh_queued_msg_send(poll_timer_expire_handler,data);
}

static void friend_k2_derivation_complete(k2_derivation_buf_t *param,void *dummy,uint8_t status)
{
    LOG_I("%s",__func__);
}

bool param_check_in_friend_request_rx_handler(upper_pdu_rx_t *ptr) 
{

    friend_request_payload_t *pdu = (friend_request_payload_t *)ptr->src;

    // PTS test: Invalid param is prohibited.  according to Mesh Spec:v1.0.1

    if(pdu->criteria.min_queue_size_log == 0x00)//MinQueueSizeLog == 0b000 is prohibited.
    {
   	    LOG_I("[PTS %s]: error min_queue_size_log\n", __func__);
        return 0;
    }
   	if(pdu->receive_delay <= 0x09)
   	{
        LOG_I("[PTS %s]: error receive_delay\n",__func__);
   	    return 0;
   	}
   	if((uint32_t)(THREE_BYTES_ENDIAN_REVERSE(pdu->poll_timeout_be)) <= (uint32_t)FRIEND_POLLTIMEOUT_LOWER_BOUND || (uint32_t)(THREE_BYTES_ENDIAN_REVERSE(pdu->poll_timeout_be)) >= (uint32_t)FRIEND_POLLTIMEOUT_UPPER_BOUND)
   	{
   	    LOG_I("[PTS %s]: error poll_timeout_be\n",__func__);
   	    return 0;
   	}
   	if(pdu->elem_nums == 0x00)
   	{
   	    LOG_I("[PTS %s]: error elem_nums\n",__func__);
   	    return 0;
   	}
    return 1;
}

void friend_request_rx_handler(upper_pdu_rx_t *ptr)
{
    LOG_I("%s:%s",__func__,"IN");
    friend_request_payload_t *pdu = (friend_request_payload_t *)ptr->src;
    uint8_t min_queue_size = 1;
    uint8_t i;

	if(param_check_in_friend_request_rx_handler(ptr) == 0)
    {
        return;
    }
	
    for(i = 0;i<pdu->criteria.min_queue_size_log;++i)
    {
        min_queue_size *= 2;
    }
    if(min_queue_size>QUEUE_LENGTH_PER_FRIEND)
    {
        //LOG
        return;
    }
    friend_list_t *friend = friend_primary_addr_search(ptr->src_addr);
    if(NULL != friend)
    {
        uint16_t pre_lpn_counter = friend->lpn_counter;
        uint16_t new_lpn_counter = co_bswap16(pdu->lpn_counter_be);
        if(pre_lpn_counter == new_lpn_counter )
        {
            return;
        }
        else
        {
            friendship_cleanup(friend);
        }
        
    }
    friend = friend_env_calloc();
    if(friend==NULL)
    {
        //LOG
        return;
    }
    INIT_ARRAY_BUF_TYPE_VAR(&friend->queue);
    friend->friendship_established = false;
    friend->receive_delay = pdu->receive_delay;
    friend->uni_addr = ptr->src_addr;
    friend->element_nums = pdu->elem_nums;
    friend->poll_timer = mesh_timer_create("poll timeout", pdMS_TO_TICKS(1000),pdFALSE,friend, poll_timer_callback);
    friend->poll_timeout = THREE_BYTES_ENDIAN_REVERSE(pdu->poll_timeout_be);
    friend->lpn_counter = co_bswap16(pdu->lpn_counter_be);
    friend->prev_friend = co_bswap16(pdu->prev_addr_be);

    int8_t rssi = (int8_t)(ptr->rssi);  //Note: ptr->rssi is uint8_t, however it is a negative number stored in ptr->rssi.

    //Please refer to Mesh Profile Spec v1.0.1  table 3.23 RSSIFactor field values
    //int16_t receive_window_factor = pdu->criteria.receive_window_factor * 0.5 + 1;
    //int16_t rssi_factor = pdu->criteria.rssi_factor * 0.5 + 1;
    //uint16_t local_delay = receive_window_factor*FRIEND_RECEIVE_WINDOW - rssi_factor * rssi;
    
	int16_t receive_window_factor = (int16_t)(pdu->criteria.receive_window_factor);
	int16_t rssi_factor = (int16_t)(pdu->criteria.rssi_factor);
	uint16_t local_delay = ((receive_window_factor+2)*FRIEND_RECEIVE_WINDOW - (rssi_factor + 2)*rssi)/2;
	
    local_delay = local_delay > 100 ? local_delay : 100;
    ble_txrx_time_t expected_tx_time = mesh_time_add(ptr->rx_time,mesh_ms_to_ble_time(local_delay));
 //   LOG_I("expected_tx_time %d,rx_time %d,delay %d",expected_tx_time.time_cnt,ptr->rx_time,mesh_ms_to_ble_time(local_delay).time_cnt);
    friend_offer_tx(ptr->src_addr, ptr->rssi, expected_tx_time,ptr->net_security);
    friend->friend_counter = friend_counter; 
    friend->security.netkey = ptr->netkey;
//
//    BX_ASSERT(hdr == NULL);

    co_list_push_back(&ptr->netkey->friend,&friend->security.hdr);
    net_key_box_t *netkey_box = CONTAINER_OF(ptr->net_security, net_key_box_t, master);
    k2_derivation_buf_t k2_param = {
        .n = netkey_box->netkey,
        .friend = {
            .friend_cnt = friend_counter,
            .lpn_cnt = friend->lpn_counter,
            .friend_addr = mesh_node_get_primary_element_addr(),
            .lpn_addr = ptr->src_addr,
        },
        .rslt = &friend->security.credentials[&ptr->netkey->key[0]==netkey_box?0:1],
        .master = false,
    };
    LOG_D("",__func__,"OUT");
    k2_derivation_start(&k2_param, friend_k2_derivation_complete);
}

static void friend_clear_confirm_tx_done_callback(void *aaa,uint8_t bbb)
{
    LOG_I("%s",__func__);
}

static void friend_clear_confirm_tx(uint16_t dst_addr,uint16_t lpn_addr,uint16_t lpn_counter,security_credentials_t *net_security)
{
    LOG_I("%s",__func__);
    control_msg_tx_param_t param= {
        .expected_tx_time = NULL,
        .dst_addr = dst_addr,
        .length = FRIEND_CLEAR_CONFIRM_PDU_LENGTH,
        .netkey_credentials = net_security,
        .opcode = FRIEND_CLEAR_CONFIRM,
        .ttl = 0x7f,//max
        .high_priority = false,
    };
    control_pdu_tx_t *pdu = control_unseg_msg_build(&param,friend_clear_confirm_tx_done_callback);
    BX_ASSERT(pdu);
    friend_clear_confirm_payload_t *ptr = (void*)pdu->payload;
    ptr->lpn_addr_be = co_bswap16(lpn_addr);
    ptr->lpn_counter_be = co_bswap16(lpn_counter);
    control_send(pdu);
}

void friend_clear_rx_handler(upper_pdu_rx_t *ptr)
{
    LOG_I("%s",__func__);
    friend_clear_payload_t *pdu = (void*)ptr->src;
    uint16_t lpn_addr = co_bswap16(pdu->lpn_addr_be);
    uint16_t lpn_counter = co_bswap16(pdu->lpn_counter_be);
    friend_list_t *friend = friend_primary_addr_search(lpn_addr);
    uint16_t modulo = (lpn_counter - friend->lpn_counter) %65536;
    if(modulo<=255)
    {
        //TODO-FRIEND  net_security assignment
        friend_clear_confirm_tx(ptr->src_addr,lpn_addr,lpn_counter,ptr->net_security);
        if(mesh_timer_cancel(friend->poll_timer))
        {
            friendship_cleanup(friend);
        }
    }
}

static friend_list_t *friend_search_by_prev_friend_addr(uint16_t addr)
{
    friend_list_t *ptr;
    FOR_EACH_ALLOCATED_ITEM(friend_list, ptr, 
        if(ptr->prev_friend == addr)
        {
            return ptr;
        }
    )
    return NULL;
}

void friend_clear_confirm_rx_handler(upper_pdu_rx_t *ptr)
{
    LOG_I("%s",__func__);
    friend_list_t *friend = friend_search_by_prev_friend_addr(ptr->src_addr);
    BX_ASSERT(friend);
    clear_repeat_timer_cancel_and_delete(friend);
    clear_procedure_timer_cancel_and_delete(friend);
}

bool address_already_in_list(uint16_t address , uint16_t* list , uint8_t count)
{
    bool retval = false;
    for(uint8_t i=0;i<count;i++)
    {
        if(address == list[i])
        {
            retval = true;
            break;
        }
    }
    return retval;
}

static void friend_subscription_list_confirm_tx_done_callback(void *aaa,uint8_t bbb)
{
    LOG_I("%s",__func__);
}

static void friend_subscription_list_confirm_tx(uint16_t dst_addr,uint8_t transaction_number,security_credentials_t *net_security)
{
    LOG_I("%s",__func__);
    control_msg_tx_param_t param= {
        .expected_tx_time = NULL,
        .dst_addr = dst_addr,
        .length = FRIEND_SUBSCRIPTION_LIST_CONFIRM_LENGTH,
        .netkey_credentials = net_security,
        .opcode = FRIEND_SUBSCRIPTION_LIST_CONFIRM,
        .ttl = 0,
        .high_priority = false,
    };
    control_pdu_tx_t *pdu = control_unseg_msg_build(&param,friend_subscription_list_confirm_tx_done_callback);
    BX_ASSERT(pdu);
    friend_sunscription_list_confirm_payload_t *ptr = (void*)pdu->payload;
    ptr->transaction_number = transaction_number;
    control_send(pdu);

    LOG_D("current sub list:");
    friend_list_t *friend = friend_primary_addr_search(dst_addr);
    for(uint8_t i=0;i<SUBSCRIPTION_LIST_SIZE_PER_FRIEND;i++)
    {
        LOG_D("0x%x",friend->subscript_list[i]);
    }
}

void friend_subscription_list_add_rx_handler(upper_pdu_rx_t *ptr)
{
    LOG_I("%s",__func__);
    friend_list_t *friend = friend_primary_addr_search(ptr->src_addr);
    uint8_t list_num = (ptr->total_length - 1 ) / 2;
    uint16_t address;
    uint8_t list_count = SUBSCRIPTION_LIST_SIZE_PER_FRIEND;
    friend_sunscription_list_add_payload_t *pdu = (void *)ptr->src;
    for(uint8_t i=0;i<list_num;i++)
    {
        address = co_bswap16(pdu->address_list_be[i]);
        //not already in list
        if(false == address_already_in_list(address , friend->subscript_list , list_count))
        {
            //search enpty location
            for(uint8_t j=0;i<list_count;j++)
            {
                if(0 == friend->subscript_list[j])
                {
                    friend->subscript_list[j] = address;
                    break;
                }
            }
        }
    }
    //reply
    friend_subscription_list_confirm_tx(ptr->src_addr , pdu->transaction_number , ptr->net_security);
}

void friend_subscription_list_remove_rx_handler(upper_pdu_rx_t *ptr)
{
    LOG_I("%s",__func__);
    friend_list_t *friend = friend_primary_addr_search(ptr->src_addr);
    uint8_t list_num = (ptr->total_length - 1 ) / 2;
    uint16_t address;
    uint8_t list_count = SUBSCRIPTION_LIST_SIZE_PER_FRIEND;
    friend_sunscription_list_add_payload_t *pdu = (void *)ptr->src;
    for(uint8_t i=0;i<list_num;i++)
    {
        address = co_bswap16(pdu->address_list_be[i]);
        //search location
        for(uint8_t j=0;j<list_count;j++)
        {
            if(address == friend->subscript_list[j])
            {
                friend->subscript_list[j] = 0;
            }
        }
    }
    //reply
    friend_subscription_list_confirm_tx(ptr->src_addr , pdu->transaction_number , ptr->net_security);
}


void friend_seg_list_cleanup(friend_list_t *env)
{
    friend_queue_list_cleanup(env,&env->seg_list);
}

void friend_update_add_to_q_for_all(uint8_t* network_id)
{
    mesh_key_refresh_phase_t key_refresh_phase;
    dm_netkey_handle_t netkey_handle ;
    bool iv_index_update ;
    
    if(MESH_CORE_SUCCESS != dm_netkey_network_id_to_handle(network_id,&netkey_handle))
    {
        LOG_I("%s -> INVALID_PARAM\n",__func__);
        return ;
    }
    dm_netkey_kr_phase_get(netkey_handle,&key_refresh_phase);
    iv_index_update = mesh_iv_update_is_processing();

    struct co_list_hdr *hdr = co_list_pick(&netkey_handle->friend);
    while(hdr)
    {
        friend_security_t *ptr = CONTAINER_OF(hdr, friend_security_t,hdr);
        friend_list_t *env  = CONTAINER_OF(ptr, friend_list_t,security);
        friend_update_add_to_q(env,key_refresh_phase == MESH_KEY_REFRESH_PHASE_2,iv_index_update);
        hdr = co_list_next(hdr);
    }
    
}


void generate_friend_credentials(void* dm_handle)
{
    dm_netkey_handle_t netkey_handle = (dm_netkey_handle_t)dm_handle;
    struct co_list_hdr *hdr = co_list_pick(&netkey_handle->friend);
//    LOG_I("!!!!friend hdr 0x%x\n",hdr);
    while(hdr)
    {
        friend_security_t *ptr = CONTAINER_OF(hdr, friend_security_t,hdr);
        friend_list_t *friend  = CONTAINER_OF(ptr, friend_list_t,security);

            k2_derivation_buf_t k2_param = {
        .n = netkey_handle->key[dm_netkey_get_update_index(netkey_handle)].netkey,
        .friend = {
            .friend_cnt = friend->friend_counter,
            .lpn_cnt = friend->lpn_counter,
            .friend_addr = mesh_node_get_primary_element_addr(),
            .lpn_addr = friend->uni_addr,
        },
        .rslt = &friend->security.credentials[dm_netkey_get_update_index(netkey_handle)],
        .master = false,
        };
        LOG_D("",__func__,"OUT");
        k2_derivation_start(&k2_param, friend_k2_derivation_complete);
        hdr = co_list_next(hdr);
    }
}

