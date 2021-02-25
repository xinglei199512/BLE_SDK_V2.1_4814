#include "network_tx_process.h"
#include "log.h"
#include "mesh_gatt.h"
#include "adv_bearer_tx.h"
#include "network_pdu_decrypt.h"
#include "co_endian.h"
#include "upper_tx_process.h"
#include "network_pdu_encrypt.h"
#include "stack_mem_cfg.h"
#include "lower_tx_process.h"
#include "adv_bearer_tx.h"
#include "adv_bearer_rx.h"
#include "mesh_queued_msg.h"
#include "mesh_core_api.h"
#include "mesh_env.h"
#include "friend.h"


static network_pdu_packet_u encrypted_tx_data;

static void network_adv_tx_timer_callback(mesh_timer_t timer);

static void network_tx_pre_process(network_pdu_tx_t **);
static void network_tx_process_complete_callback(network_pdu_tx_t **ptr,void *param,uint8_t status);
QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(network_tx_async_process, NETWORK_TX_PROCESS_BUF_QUEUE_LENGTH,
    network_pdu_tx_t *, network_tx_pre_process, network_tx_process_complete_callback);

DEF_ARRAY_BUF(tx_pdu_buf, network_pdu_tx_t, NETWORK_TX_PDU_BUF_SIZE);
DEF_ARRAY_BUF(network_adv_tx_buf,network_adv_tx_t,NETWORK_ADV_TX_BUF_SIZE);

static uint8_t network_max_used_num = 0;

static network_adv_tx_t *network_adv_tx_buf_alloc(uint8_t pkt_type)
{
    network_adv_tx_t *ptr = array_buf_alloc(&network_adv_tx_buf);
    uint8_t tmp_size = array_buf_available_size(&network_adv_tx_buf);
    if((NETWORK_ADV_TX_BUF_SIZE - tmp_size) > network_max_used_num)
    {
        network_max_used_num = NETWORK_ADV_TX_BUF_SIZE - tmp_size;
        LOG(LOG_LVL_INFO,"adv_tx buf remain %d\n",network_max_used_num);
    }
    BX_ASSERT(NULL != ptr);
    bearer_adv_tx_timer_create("adv_tx", &ptr->adv, false, network_adv_tx_timer_callback);
    return ptr;
}

static network_adv_tx_t *network_adv_pkt_prepare(uint8_t *data,uint16_t length,uint8_t pkt_type,uint16_t dst_addr,uint8_t ctl)
{
    network_adv_tx_t *adv_tx = network_adv_tx_buf_alloc(pkt_type);
    BX_ASSERT(adv_tx);
    adv_tx->adv.data_length = length + DATA_OFFSET_IN_BEARER;
    adv_tx->adv.adv_data[0] = length + 1;
    adv_tx->adv.adv_data[1] = MESH_MESSAGE_AD_TYPE;
    memcpy(adv_tx->adv.adv_data + DATA_OFFSET_IN_BEARER,data,length);
    adv_tx->adv.scan_rsp = NULL;
    adv_tx->adv.pkt_type = pkt_type;
    adv_tx->dst_addr = dst_addr;
    adv_tx->ctl = ctl;
    return adv_tx;
}

void network_adv_tx_buf_retain(network_adv_tx_t *ptr)
{
    array_buf_retain(&network_adv_tx_buf, (void *)ptr);
}

bool network_adv_tx_buf_release(network_adv_tx_t *ptr)
{
//    LOG(LOG_LVL_INFO,"release 0x%x\n",ptr);
    if(array_buf_release(&network_adv_tx_buf,ptr))
    {
        mesh_timer_delete(ptr->adv.timer);
        ptr->adv.timer = NULL;
        return true;
    }else
    {
        return false;
    }
}

bool network_adv_tx_cancel(network_adv_tx_t *ptr)
{
    if(bearer_adv_tx_timer_cancel(&ptr->adv))
    {
        bool released = network_adv_tx_buf_release(ptr);
        BX_ASSERT(released);
        return true;
    }else
    {
        return false;
    }
}

static void network_adv_tx_msg_send(network_adv_tx_t *adv_tx)
{
    ++adv_tx->param.count;
    bearer_adv_tx_msg_send(&adv_tx->adv,adv_tx->param.high_priority);
}

network_pdu_tx_t *tx_pdu_buf_alloc(void)
{
    return array_buf_alloc(&tx_pdu_buf);
}

network_pdu_tx_t *network_tx_pdu_head_build(network_pdu_packet_head_t *head)
{
    network_pdu_tx_t *ptr = tx_pdu_buf_alloc();
    BX_ASSERT(ptr);
    ptr->src.head = *head;
    return ptr;
}
 
void tx_pdu_buf_release(network_pdu_tx_t *ptr)
{
    array_buf_release(&tx_pdu_buf,ptr);
}

static void network_tx_process_complete_callback(network_pdu_tx_t **ptr,void *param,uint8_t status)
{
    tx_pdu_buf_release(*ptr);
}

void network_pkt_local_tx(uint8_t *data,uint8_t length,uint16_t dst_addr)
{
    network_pdu_rx_t *rx_pdu = network_pdu_alloc_and_fill(data,length,NORMAL_NETWORK_PKT_RX,LOCAL_NETWORK_INTERFACE_RX);
    if(rx_pdu == NULL)
    {
        LOG(LOG_LVL_WARN,"local:no avaiable rx network pdu buf\n");
    }else
    {
        mesh_msg_rx(rx_pdu);
    }
}

void network_pkt_gatt_tx(uint8_t *data,uint16_t gatt_mask,uint8_t pkt_type,uint16_t length,uint16_t dst_addr)
{
    uint8_t i;
    for(i=0;i<MESH_GATT_CONNECTION_COUNT_MAX;++i)
    {
        if(gatt_mask&1<<i)
        {
            if(pkt_type == NON_ADV_PROXY_CONFIG_PKT)
            {
                mesh_gatt_proxy_pdu_tx(MESH_GATT_PDU_TYPE_PROXY_CONFIG,data ,length ,dst_addr);
            }
            else
            {
                mesh_gatt_proxy_pdu_tx(MESH_GATT_PDU_TYPE_NETWORK_PDU,data,length ,dst_addr);
            }
        }
    }
}

static bool network_tx_local_check(uint16_t dst_addr)
{
    mesh_elmt_t *elmt = get_mesh_elmt();

    if(IS_UNICAST_ADDR(dst_addr))
    {
        if(get_elmt_by_uni_addr(dst_addr))
        {
            return true;
        }
    }
    else if(dst_addr == MESH_ALL_NODES_ADDR)
    {
        return true;
    }
    else
    {
        //subscription_list
        uint8_t i , j;
        for(i=0;i<get_element_num();++i)//every element in node
        {
            struct co_list_hdr *hdr = co_list_pick(&elmt[i].model_list);
            while(hdr)
            {
                model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);//every model in element
                for(j=0;j<model->subscription_list_size;j++)//every subscribe in model
                {
                    //group address
                    if(model->subscription_list[j].is_virt == false)
                    {
                        if(model->subscription_list[j].addr.addr == dst_addr)
                        {
                            return true;
                        }
                    }
                    else//virtual addr
                    {
                        //TODO
                    }
                }
                hdr = co_list_next(hdr);
            }
        }
    }
    return false;
}

static void network_tx_bearer_get(uint16_t dst_addr,uint8_t pkt_type,bool *local,bool *adv,uint32_t *gatt_mask)
{
    if(NON_ADV_PROXY_CONFIG_PKT == pkt_type)
    {
        *local = false;
        *adv = false;
        *gatt_mask = ~0;
    }else if(FRIEND_QUEUE_ADV_PKT == pkt_type)
    {
        *local = false;
        *gatt_mask = 0;
        *adv = true;
   }else
   {
        if(pkt_type == UPPER_LAYER_ADV_PKT && network_tx_local_check(dst_addr))
        {
             LOG(LOG_LVL_WARN,"tx loop!!!\n");
            *local = true;
        }
        else
        {
            *local = false;
        }
       *adv = true;
       *gatt_mask = ~0;
   }
}

static uint8_t network_total_length_get(uint8_t ctl,uint8_t pdu_length)
{
    return sizeof(network_pdu_packet_head_t) + pdu_length + (ctl ? CONTROL_MSG_NETMIC_SIZE : ACCESS_MSG_NETMIC_SIZE);
}

static void network_pkt_tx_start(network_pdu_packet_u *encrypted,network_pdu_tx_t *tx)
{
    bool local,adv;
    uint32_t gatt_mask;
    uint16_t dst_addr = co_bswap16(tx->src.head.dst_be);
    uint8_t total_length = network_total_length_get(tx->src.head.ctl,tx->src.lower_pdu_length);
    network_tx_bearer_get(dst_addr,tx->pkt_type,&local,&adv,&gatt_mask);
    network_adv_tx_t *adv_tx = network_adv_pkt_prepare(encrypted->buf,total_length,tx->pkt_type,dst_addr,tx->src.head.ctl);
    BX_ASSERT(adv_tx);
    uint32_t delay_ticks;
    switch(tx->pkt_type)
    {
    case UPPER_LAYER_ADV_PKT:
        delay_ticks = upper_pkt_tx_start(dst_addr,tx->src.head.ctl,&local,&adv,&gatt_mask,adv_tx);
    break;
    case SEGMENT_ACK_ADV_PKT:
        delay_ticks = segment_pkt_tx_start(dst_addr,&local,&adv,&gatt_mask,adv_tx);
    break;
    case RELAY_ADV_PKT:
        delay_ticks = relay_pkt_tx_start(dst_addr,&local,&adv,&gatt_mask,adv_tx);
    break;
    case NON_ADV_PROXY_CONFIG_PKT:
        adv_tx->param.high_priority = false;
        adv_tx->param.repeats = 0;
        delay_ticks = 0;
     break;
    case FRIEND_QUEUE_ADV_PKT:
        delay_ticks = friend_queue_pkt_tx_start(dst_addr,&local,&adv,&gatt_mask,adv_tx);
 //       LOG_D("delay_ms %d\n",delay_ms);
    break;
    default:
        BX_ASSERT(0);
    break;
    }
    adv_tx->param.count = 0;
    if(local)
    {
        network_pkt_local_tx(encrypted->buf,total_length,dst_addr);
    }
    if(gatt_mask)
    {
        network_pkt_gatt_tx(encrypted->buf,gatt_mask,tx->pkt_type,total_length,dst_addr);
    }
    if(adv)
    {
        if(delay_ticks)
        {
            adv_tx->param.delayed_start = true;
            mesh_timer_change_period(adv_tx->adv.timer,delay_ticks);
        }else
        {
            adv_tx->param.delayed_start = false;
            network_adv_tx_msg_send(adv_tx);
        }
    }else
    {
        network_adv_tx_buf_release(adv_tx);
    }
}

void network_adv_retransmit_for_friend_q_element(network_adv_tx_t *adv,uint32_t delay_ticks)
{
    BX_ASSERT(delay_ticks);
    adv->param.delayed_start = true;
    mesh_timer_change_period(adv->adv.timer,delay_ticks);
}

static void network_adv_tx_timer_expire_handler(mesh_adv_tx_t *adv)
{
    network_adv_tx_t *ptr = CONTAINER_OF(adv, network_adv_tx_t, adv);
    if(ptr->param.delayed_start)
    {
        ptr->param.delayed_start = false;
        network_adv_tx_msg_send(ptr);
//        LOG_D("delayed_start 0x%x\n",ptr);

    }else
    {
//        LOG_D("not_delayed 0x%x\n",ptr);
        bool adv_continue;
        switch(adv->pkt_type)
        {
        case UPPER_LAYER_ADV_PKT:
            adv_continue = upper_tx_env_adv_tx_timer_expire_handler(ptr);
        break;
        case SEGMENT_ACK_ADV_PKT:
        case RELAY_ADV_PKT:
            adv_continue = true;
        break;
        default:
            BX_ASSERT(0);
        break;
        }
        if(adv_continue)
        {
            network_adv_tx_msg_send(ptr);
        }else
        {
            bool released = network_adv_tx_buf_release(ptr);
            BX_ASSERT(released);
        }
    }
}

static void network_adv_tx_timer_callback(mesh_timer_t timer)
{
    mesh_adv_tx_t *adv = mesh_timer_get_associated_data(timer);
    mesh_queued_msg_send((void (*)(void *))network_adv_tx_timer_expire_handler,adv);
}

static void network_tx_encrypt_done(network_pdu_packet_u *encrypted)
{
    network_pdu_tx_t **ptr = queued_async_process_get_current(&network_tx_async_process);
    network_pdu_tx_t *tx_pdu = *ptr;
    encrypted->pkt.nid = tx_pdu->netkey_credentials->nid;
    encrypted->pkt.ivi = tx_pdu->iv_index & 0x1;
    network_pkt_tx_start(encrypted,tx_pdu);
    queued_async_process_end(&network_tx_async_process,NULL,0);
}

static void network_tx_pre_process(network_pdu_tx_t **ptr)
{
    network_pdu_tx_t *pdu = *ptr;
    network_pdu_encrypt_start(&pdu->src,pdu->iv_index, pdu->netkey_credentials, network_tx_encrypt_done,pdu->pkt_type,&encrypted_tx_data);
}

void network_tx_process_start(network_pdu_tx_t *pdu)
{
    queued_async_process_start(&network_tx_async_process,&pdu,NULL);
}

void network_tx_on_adv_start_callback(mesh_adv_tx_t *adv)
{
    network_adv_tx_t *ptr = CONTAINER_OF(adv,network_adv_tx_t, adv);
    bool adv_cnt_expire = ptr->param.count > ptr->param.repeats;
    bool force_stop;
    uint16_t interval_ms;
    switch(adv->pkt_type)
    {
    case UPPER_LAYER_ADV_PKT:
        force_stop = upper_adv_tx_start_hook(ptr,adv_cnt_expire,&interval_ms);
    break;
    case SEGMENT_ACK_ADV_PKT:
        force_stop = false;
        interval_ms = segment_ack_adv_tx_interval_get(ptr->dst_addr);
    break;
    case RELAY_ADV_PKT:
        force_stop = false;
        interval_ms = relay_adv_tx_interval_get(ptr->dst_addr);
    break;
    case FRIEND_QUEUE_ADV_PKT:
        force_stop = true;
    break;
    default:
        BX_ASSERT(0);
    break;
    }
//    LOG(LOG_LVL_WARN,"on_adv_start : 0x%x\n",ptr);

    if(adv_cnt_expire || force_stop)
    {
        bool released = network_adv_tx_buf_release(ptr);
        if(adv->pkt_type !=  FRIEND_QUEUE_ADV_PKT) BX_ASSERT(released);
    }else
    {
        bearer_adv_tx_timer_start(adv,interval_ms);
    }
}


