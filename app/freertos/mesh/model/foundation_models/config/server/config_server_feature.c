
#include "osapp_config.h"
#include "mesh_model.h"
#include "mesh_core_api.h"
#include "access_rx_process.h"
#include "foundation_common.h"
#include "foundation_msg_handler.h"
#include "config_server_events_api.h"
#include "config_server_feature.h"
#include "heartbeat.h"
#include "config_server.h"
#include "low_power.h"
#include "friend.h"

static void config_friend_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_friend_status_tx_done\n");
}
void config_friend_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    mesh_core_params_t param;
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_FRIENT           , &param);

    LOG(3, "config_friend_get_rx:%x\n", param.friend);
    fd_model_two_octets_status_tx(access_get_pdu_netkey_global_index(pdu), access_get_pdu_src_addr(pdu),&pdu->uppdu->rx_time,
            config_friend_status_tx_done, (uint8_t *)&param, 1, Config_Friend_Status);
}
void config_friend_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t *access = access_get_pdu_payload(pdu);
    uint8_t friend_temp = *(uint8_t *)(access + 2);

    mesh_core_params_t param;
    param.friend = friend_temp;
    mesh_core_params_set(MESH_CORE_PARAM_FEATURE_FRIENT           , &param);

    LOG(3, "config_friend_set_rx:%x\n", param.friend);
    fd_model_two_octets_status_tx(access_get_pdu_netkey_global_index(pdu), access_get_pdu_src_addr(pdu),&pdu->uppdu->rx_time,
            config_friend_status_tx_done, (uint8_t *)&param, 1, Config_Friend_Status);

    if(param.friend != MESH_FEATURE_DISABLED)
        send_heartbeat_massage((uint16_t)(4 << 8));
    else
        send_heartbeat_massage((uint16_t)0);
}

static void config_proxy_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_proxy_status_tx_done\n");
}

void config_proxy_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    config_proxy_param_t param;
    //get feature
    mesh_core_params_t param_feature_proxy ;
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_PROXY           , &param_feature_proxy);
    //set value
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    param.proxy.proxy_state                     = param_feature_proxy.proxy;
    fd_model_two_octets_status_response_tx(pdu,config_proxy_status_tx_done,(uint8_t *)&param ,sizeof(config_proxy_param_t),Config_GATT_Proxy_Status);
    LOG(3,"config_proxy_get_rx\n");
}

void config_proxy_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    config_proxy_param_t param;
    mesh_core_params_t param_core_proxy;
    uint8_t * access = access_get_pdu_payload(pdu);
    memcpy(&param , access + 2 , 1);
    //set to core
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);

    param_core_proxy.proxy = (mesh_feature_stat_t)param.proxy.proxy_state;

    LOG(3,"config_proxy_set_rx:%x\n", param_core_proxy.proxy);

    if(param_core_proxy.proxy > 0x2)
        return;

    mesh_core_params_set(MESH_CORE_PARAM_FEATURE_PROXY, &param_core_proxy);
    //send status message
    fd_model_two_octets_status_response_tx(pdu,config_proxy_status_tx_done,(uint8_t *)&param ,sizeof(config_proxy_param_t),Config_GATT_Proxy_Status);
    send_heartbeat_massage((uint16_t)((param_core_proxy.proxy << 1) << 8));
    //user callback
    config_server_evt_param_t evt_param = {
        .gatt_proxy_set.enabled = param.proxy.proxy_state,
    };
    config_server_evt_act(CONFIG_SERVER_EVT_GATT_PROXY_SET,&evt_param);
}

static void config_relay_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_relay_status_tx_done\n");
}

void config_relay_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    config_relay_param_t param;
    //get feature
    mesh_core_params_t param_feature_relay , param_property_relay_transmit;
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_RELAY           , &param_feature_relay);
    mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_RELAY_TRANSMIT , &param_property_relay_transmit);
#if 0
    param_feature_relay.relay = relay;
    param_property_relay_transmit.relay_transmit.count = count;
    param_property_relay_transmit.relay_transmit.interval_steps = interval_steps;
#endif
    //set value
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    param.relay                     = param_feature_relay.relay;
    param.retransmit.count          = param_property_relay_transmit.relay_transmit.count;
    param.retransmit.interval_steps = param_property_relay_transmit.relay_transmit.interval_steps;
    fd_model_two_octets_status_response_tx(pdu,config_relay_status_tx_done,(uint8_t *)&param ,sizeof(config_relay_param_t),Config_Relay_Status);

    LOG(3,"config_relay_get_rx\n");
}

void config_relay_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    config_relay_param_t param;
    mesh_core_params_t param_core_relay , param_core_transmit;
    uint8_t * access = access_get_pdu_payload(pdu);
    memcpy(&param , access + 2 , 2);

    if(param.relay > 0x02)
        return;

    //set to core
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    param_core_relay.relay                              = param.relay;
    param_core_transmit.relay_transmit.count            = param.retransmit.count;
    param_core_transmit.relay_transmit.interval_steps   = param.retransmit.interval_steps;
#if 0
    if(param_core_relay.relay != 0) {

        relay = param_core_relay.relay;
        count = param_core_transmit.relay_transmit.count;
        interval_steps = param_core_transmit.relay_transmit.interval_steps;
    }
#endif

    mesh_core_params_set(MESH_CORE_PARAM_FEATURE_RELAY           , &param_core_relay);
    mesh_core_params_set(MESH_CORE_PARAM_PROPERTY_RELAY_TRANSMIT , &param_core_transmit);
    //config_relay_get_rxsend status message
    fd_model_two_octets_status_response_tx(pdu,config_relay_status_tx_done,(uint8_t *)&param ,sizeof(config_relay_param_t),Config_Relay_Status);

#if 1
    send_heartbeat_massage((uint16_t)((param_core_relay.relay) << 8));
    LOG(3,"test config_relay_set_rx relay:%d\n", param_core_relay.relay);
#endif

    //user callback
    config_server_evt_param_t evt_param = {

        .relay_set.enabled = param.relay,
        .relay_set.retransmit_count = param.retransmit.count,
        .relay_set.interval_steps = param.retransmit.interval_steps,
    };
    config_server_evt_act(CONFIG_SERVER_EVT_RELAY_SET,&evt_param);
}

static void config_low_power_node_polltimeout_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_low_power_node_polltimeout_status_tx_done\n");
}

void config_low_power_node_polltimeout_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t *buff = (uint8_t *)pdu->access_payload;
    uint16_t lpn_addr = (buff[2]) | (buff[3]<<8);
    uint32_t poll_timeout = get_lp_polltimeout_value();
    uint8_t  send_buf[5];
    if(NULL == friend_search_for_caching_msg(lpn_addr))
    {
        poll_timeout = 0x0;
        LOG(3,"config_low_power_node_polltimeout_get_rx \n");

    }
    send_buf[0] = lpn_addr & 0xFF;
    send_buf[1] = lpn_addr >> 8;
    send_buf[2] = (poll_timeout & 0xFF0000) >> 0;
    send_buf[3] = (poll_timeout & 0x00FF00) >> 8 ;
    send_buf[4] = (poll_timeout & 0x0000FF) >> 16 ;

    LOG(3,"config_low_power_node_polltimeout_get_rx \n");
    fd_model_two_octets_status_response_tx(pdu,config_low_power_node_polltimeout_status_tx_done,send_buf,5,Config_Low_Power_Node_PollTimeout_Status);
}


