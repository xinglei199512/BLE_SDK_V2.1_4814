#define LOG_TAG        "network_rx_process.c"
#define LOG_LVL        LVL_DBG
#include "bx_log.h"


#include "network_rx_process.h"
#include "network_pdu_decrypt.h"
#include "network_msg_cache.h"
#include "log.h"
#include "aes_ccm_cmac.h"
#include "co_endian.h"
#include "mesh_gatt_proxy.h"
#include "adv_bearer_rx.h"

#include "mesh_core_api.h"
#include "mesh_gatt.h"
#include "mesh_env.h"
#include "stack_mem_cfg.h"
#include "lower_rx_process.h"
#include "heartbeat.h"
#include "friend.h"
#include "low_power.h"

#if MESH_TOOLS_TTL_TEST
#include "mesh_ttl_test.h"
#endif
extern void relay_pkt_send(network_pdu_decrypt_callback_param_t *param,uint8_t total_length);

void network_rx_process_callback(network_pdu_rx_t **ptr,void *dummy,uint8_t status);
static void network_rx_pre_process(network_pdu_rx_t **);
QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(network_rx_async_process, NETWORK_RX_PROCESS_BUF_QUEUE_LENGTH,
    network_pdu_rx_t *, network_rx_pre_process, network_rx_process_callback);

static bool relay_condition_check(network_pdu_packet_u *ptr)
{
    /*
     * <1>.  If the message delivered from the advertising bearer is processed by the lower transport layer,
     * the Relay feature is supported and enabled, the TTL field has a value of 2 or greater,and the destination
     * is not a unicast address of an element on this node, then the TTL field value shall be decremented by 1,
     * the Network PDU shall be tagged as relay, and the Network PDU shall be retransmitted to all network interfaces
     * connected to the advertising bearer.
     *
     * <2>. If the message delivered from the GATT bearer is processed by the lower transport layer, and the Proxy feature is supported and enabled,
     * and the TTL field has a value of 2 or greater, and the destination is not a unicast address of an element on this node,
     * then the TTL field value shall be decremented by 1, and the Network PDU shall be retransmitted to all network interfaces.
     * */
    uint8_t ttl = ptr->pkt.ttl;
    bool state = false;
    //get feature
    mesh_core_params_t param_relay;
    mesh_core_params_t param_proxy;
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_RELAY , &param_relay);
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_PROXY , &param_proxy);


    if( !get_elmt_by_uni_addr(co_bswap16(ptr->pkt.dst_be))//1. uni dst not my self
        && ttl >=2 //2. ttl >= 2
        && (       //3.
              (param_relay.relay == MESH_FEATURE_ENABLED)//relay enabled
            || ( (param_proxy.proxy == MESH_FEATURE_ENABLED)//proxy enbled
                  && mesh_gatt_connect_num_get() )//connect num >0
            )
        )
    {
        state = true;
    }

    return state;
}


static bool dst_addr_match(uint16_t dst_addr)
{
    mesh_elmt_t *elmt = get_mesh_elmt();
    //TODO unicast virtual group addr
    if(dst_addr && (dst_addr == get_heartbeat_dst_addr()) && ((dst_addr & 0xc000) != 0x8000))
        return true;


    //special address handle
    mesh_core_params_t param;
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_PROXY , &param);
    if((dst_addr == MESH_ALL_PROXIES_ADDR) && (MESH_FEATURE_ENABLED == param.proxy))
    {
        return true;
    }
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_FRIENT , &param);
    if((dst_addr == MESH_ALL_FRIENDS_ADDR) && (MESH_FEATURE_ENABLED == param.friend))
    {
        return true;
    }
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_RELAY , &param);
    if((dst_addr == MESH_ALL_RELAYS_ADDR) && (MESH_FEATURE_ENABLED == param.relay))
    {
        return true;
    }
    if(dst_addr == MESH_ALL_NODES_ADDR)
    {
        return true;
    }

    
    if(IS_UNICAST_ADDR(dst_addr))
    {
        if(get_elmt_by_uni_addr(dst_addr))
        {
            return true;
        }
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

static bool network_is_rx_self_pkt_from_air(uint8_t from,network_pdu_decrypt_callback_param_t *param)
{
    uint16_t src_addr = network_pdu_get_src_addr(param);
    if(from == LOCAL_NETWORK_INTERFACE_RX)
    {
        LOG(LOG_LVL_WARN,"rx loop  pkt\n");
        return false;
    }
    if(IS_UNICAST_ADDR(src_addr))
    {
        if(get_elmt_by_uni_addr(src_addr))
        {
            return true;
        }
    }
    return false;
}

static void network_decrypt_done(uint8_t status,network_pdu_decrypt_callback_param_t *param)
{
    //LOG_D("%s,%s",__func__,(status == DECRYPT_SUCCESSFUL)?"SUCCESS":"FAIL");
    network_pdu_rx_t **ptr = queued_async_process_get_current(&network_rx_async_process);
    network_pdu_rx_t *pdu = *ptr;
    if(status == DECRYPT_SUCCESSFUL)
    {
        if(pdu->type == PROXY_CONFIG_NETWORK_PKT_RX)
        {
            mesh_gatt_proxy_config_pdu_decrypt_callback(param,pdu->src.length,pdu->from);
            
        }else
        {
            bool low_power_mode = low_power_mode_status_get();
            if(network_is_rx_self_pkt_from_air(pdu->from, param))
            {
                LOG(LOG_LVL_INFO,"drop self pkt\n");
            }
            else  if(low_power_mode && !lpn_is_friend_security(param->net_security))
            {
                    LOG(LOG_LVL_WARN,"not friend pkt\n");
            }
            else if(network_msg_cache_search(&param->decrypted)==false
                 #if MESH_TOOLS_TTL_TEST
                 && network_rx_ttl_check(pdu,&param->decrypted)
                 #endif
                 )
            {
                if(pdu->from != ADV_BEARER_RX && pdu->from != LOCAL_NETWORK_INTERFACE_RX)
                {
                    mesh_gatt_proxy_gatt_list_filter_set(param,pdu->from);
                }
                if(network_msg_cache_full())
                {
                    network_msg_cache_remove_oldest();
                }
                network_msg_cache_add(&param->decrypted);
                if(relay_condition_check(&param->decrypted) && pdu->from != LOCAL_NETWORK_INTERFACE_RX)
                {
                    relay_pkt_send(param,pdu->src.length);
                }
                uint16_t dst_addr = co_bswap16(param->decrypted.pkt.dst_be);
                friend_list_t *friend = NULL;
                if(param->decrypted.pkt.ttl >= 2)
                {    
                    friend = friend_search_for_caching_msg(dst_addr);
                }
                bool local_use = dst_addr_match(dst_addr);
//                LOG(LOG_LVL_INFO,"dst:%d local_use :%d  0x%x\n",dst_addr,local_use,friend);
                if(local_use || friend)
                {
                    lower_transport_rx(param,pdu->src.length,pdu->from,pdu->rx_time,pdu->rssi,friend,local_use);
                }else {
                    LOG(3, "network_decrypt_done:%x\n", dst_addr);
                }
            }else
            {
                LOG(LOG_LVL_INFO,"already in network cache\n");
            }
        }
    }
    else
    {
        LOG(LOG_LVL_INFO,"network decrypt fail\n");
    }
    queued_async_process_end(&network_rx_async_process,NULL,status);
}

static void network_rx_pre_process(network_pdu_rx_t **ptr)
{
    network_pdu_rx_t *pdu = *ptr;
    network_pdu_decrypt_start(&pdu->src,pdu->type,network_decrypt_done);
}

void network_rx_process_start(network_pdu_rx_t *pdu)
{
    queued_async_process_start(&network_rx_async_process, &pdu,NULL);
}
