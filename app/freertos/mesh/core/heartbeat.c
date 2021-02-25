/**
 ****************************************************************************************
 *
 * @file   heartbeat_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-17 11:32
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "heartbeat.h"
#include "upper_pdu.h"
#include "mesh_core_api.h"
#include "timer_wrapper.h"
#include "access_tx_process.h"
#include "control_tx_process.h"
#include "app_keys_dm.h"
#include "generic_common.h"
#include "mesh_env.h"
#include "network_keys_dm.h"

static heartbeat_publication_server_t pub_server;
static heartbeat_subscription_server_t sub_server;
static mesh_timer_t pub_timer = NULL;
static mesh_timer_t sub_timer = NULL;

uint16_t get_heartbeat_dst_addr(void)
{
    return sub_server.destination_addr;
}

uint8_t heartbeat_calculate_16bit_to_8bit(uint16_t value_16)
{
    uint8_t value_8 = 0;
    if(value_16 == 0xffff) {
        value_8 = 0xff;
    }else {
        while(value_16) {
            value_8 += 1;
            value_16 >>= 1;
        }
    }

    return value_8;
}

uint32_t heartbeat_calculate_8bit_to_16bit(uint8_t value_8)
{
    uint32_t value_32 = 1;

    if(value_8 == 0xff)
        value_32 = 0xffff;
    else {
        for(uint8_t i = 0; i < value_8 - 1; i++) {
            value_32 *= 2;
        }
    }
    return value_32;
}

void heartbeat_handle_sub_timer(mesh_timer_t thandle)
{
    heartbeat_subscription_server_t *server = (heartbeat_subscription_server_t *)mesh_timer_get_associated_data(thandle);

    if(sub_server.sub_count != 0xffff)
        sub_server.sub_count++;

    LOG(3,"heartbeat_handle_sub_timer sub_count:%x\n", server->sub_count);

    if(sub_timer) {
        mesh_timer_delete(sub_timer);
        sub_timer = NULL;
    }
}
void heartbeat_start_subscription_timer(void) 
{
    uint32_t seconds;
    if((sub_server.destination_addr == 0) || (sub_server.destination_addr & 0xc000 == 0x8000))
        goto out;

    if(sub_server.sub_periodlog == 0)
        goto out;

    seconds = heartbeat_calculate_8bit_to_16bit(sub_server.sub_periodlog);
    LOG(3, "heartbeat_start_subscription_timer second:%x\n", seconds);


    if(sub_timer) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        mesh_timer_change_period(sub_timer, pdS_TO_TICKS(seconds));
    }else {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        sub_timer = mesh_timer_create(
                "sub_timer", pdS_TO_TICKS(seconds), pdFALSE, (void *)&sub_server, heartbeat_handle_sub_timer);
        if(sub_timer != NULL)
            mesh_timer_start(sub_timer);
    }

    return;
out:
    if(sub_timer) {
        mesh_timer_delete(sub_timer);
        sub_timer = NULL;
    }
}
static void heartbeat_server_set_hops(uint8_t hops)
{
    if(sub_server.minhops == 0)
        sub_server.minhops = hops;
    else if(sub_server.minhops > hops)
        sub_server.minhops = hops;

    if(sub_server.maxhops == 0)
        sub_server.maxhops = hops;
    else if(sub_server.maxhops < hops)
        sub_server.maxhops = hops;

    if(sub_server.minhops > 0x7F)
        sub_server.minhops = 0;

    if(sub_server.maxhops > 0x7F)
        sub_server.maxhops = 0;
}

static void deal_heartbeat_massage(uint8_t *payload, uint16_t src_addr, uint16_t dst_addr, uint8_t rxttl)
{
    uint8_t hops;
    heartbeat_msg_t *msg = (heartbeat_msg_t *)payload;
    if(sub_timer == NULL) {
        LOG(3, "not in periodlog recv heartbeat msg\n");
        return;
    }

    hops =  msg->initTTL - rxttl + 1;

    LOG(3, "src_addr:%x %x dst_addr:%x %x hops:%d\n", 
            sub_server.source_addr, src_addr, sub_server.destination_addr, dst_addr, hops);
    if(sub_server.source_addr != src_addr || sub_server.destination_addr != dst_addr) {
        return;
    }
#if 1
    if(sub_server.sub_count != 0xffff)
        sub_server.sub_count++;
#endif

    LOG(3,"deal_heartbeat_massage sub_count:%x\n", sub_server.sub_count);

    heartbeat_server_set_hops(hops);
    sub_server.destination_addr = src_addr;
    sub_server.source_addr = dst_addr;

    heartbeat_start_subscription_timer();
}

void heartbeat_rx_handler(upper_pdu_rx_t *ptr)
{
    deal_heartbeat_massage(ptr->src, ptr->src_addr, ptr->dst_addr, ptr->ttl);
}


static void heartbeat_handle_pub_timer(mesh_timer_t thandle)
{
    heartbeat_publication_server_t *server = (heartbeat_publication_server_t *)mesh_timer_get_associated_data(thandle);
    LOG(3,"heartbeat_handle_pub_timer pub_count:%d\n", server->pub_count);
    if(pub_timer) {
        mesh_timer_delete(pub_timer);
        pub_timer = NULL;
    }
    heartbeat_start_publication_timer();
}

void send_heartbeat_massage_cb(void *param, uint8_t status)
{
    LOG(3, "send_heartbeat_massage_cb status:%x\n", status);
}

void stop_heartbeat_massage(void)
{
    if(pub_timer)
    mesh_timer_stop(pub_timer);
}


void send_heartbeat_massage(uint16_t features)
{
    uint16_t src_addr = mesh_node_get_primary_element_addr();
    control_msg_tx_param_t param;
    dm_netkey_handle_t netkey_handle;
    if(pub_server.features == 0 || pub_server.destination_addr == 0)
    {
        return;
    }

    param.opcode = 0x0A;
    param.dst_addr = pub_server.destination_addr;
    param.ttl = pub_server.ttl;
   // param.p_buffer = (uint8_t *)&msg;
    param.length = sizeof(heartbeat_msg_t);
    dm_netkey_index_to_netkey_handle(pub_server.netkeyindex, &netkey_handle);
    dm_netkey_get_netkey_tx_credentials(netkey_handle,&param.netkey_credentials);
    LOG(3, "send_heartbeat_massage_temp dst_addr:%x feature:%x\n", pub_server.destination_addr, pub_server.features);

    //control_model_message_send(&ptr, src_addr, pub_server.destination_addr, pub_server.ttl, false, pub_server.netkeyindex);
    control_pdu_tx_t *pdu = control_unseg_msg_build(&param,send_heartbeat_massage_cb);
    BX_ASSERT(pdu);
    heartbeat_msg_t *msg = (void*)pdu->payload;
    msg->RFU = 0;
    msg->initTTL = pub_server.ttl;
    msg->features = features;
    control_send(pdu);
}

void heartbeat_start_publication_timer(void) 
{
    uint32_t seconds;
    mesh_core_params_t param_core_relay;

    if((pub_server.destination_addr == 0) || (pub_server.destination_addr & 0xc000 == 0x8000))
        goto out;

    if(pub_server.pub_periodlog == 0)
        goto out;

    if(pub_server.pub_count == 0)
        goto out;

    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_RELAY, &param_core_relay);

    LOG(3, "heartbeat_start_publication_timer feature:%x relay:%x\n", pub_server.features, param_core_relay.relay);

    if((pub_server.features & 0x1)  && (param_core_relay.relay == 0))
        pub_server.features &= 0xe;

    if(param_core_relay.relay == 0) {
        send_heartbeat_massage(pub_server.features);
        return;
    }

    seconds = heartbeat_calculate_8bit_to_16bit(pub_server.pub_periodlog);

    if(pub_server.pub_count != 0xffff)
        pub_server.pub_count--;

    send_heartbeat_massage(pub_server.features);

    if(pub_timer) {
        mesh_timer_change_period(pub_timer, pdS_TO_TICKS(seconds));
    }else {
        pub_timer = mesh_timer_create(
                "pub_timer", pdS_TO_TICKS(seconds), pdFALSE, (void *)&pub_server, heartbeat_handle_pub_timer);
        if(pub_timer != NULL)
            mesh_timer_start(pub_timer);
    }
    LOG(3, "heartbeat_start_publication_timer pub_count:%d seconds:%x\n", pub_server.pub_count, seconds);
    return;
out:
    if(pub_timer) {
        mesh_timer_delete(pub_timer);
        pub_timer = NULL;
    }
}

void heartbeat_server_init(void)
{
    memset(&pub_server, 0, sizeof(heartbeat_publication_server_t));
    memset(&sub_server, 0, sizeof(heartbeat_subscription_server_t));
    pub_server.features = 0x3;
    /* TODO init*/
    //heartbeat_start_subscription_timer();
    //heartbeat_start_publication_timer();
}

