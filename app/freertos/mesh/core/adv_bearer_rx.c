#define LOG_TAG        "adv_bearer_rx.c"
#define LOG_LVL        LVL_DBG
#include "bx_log.h"


#include <string.h>
#include "network_pdu.h"
#include "adv_bearer_rx.h"
#include "stack_mem_cfg.h"
#include "network_rx_process.h"
#include "log.h"
#include "adv_mesh_msg_cache.h"
#include "beacon.h"
#include "mesh_reset_database.h"
#include "provision.h"

DEF_ARRAY_BUF(rx_pdu_buf,network_pdu_rx_t,NETWORK_PDU_RX_BUF_SIZE);

static network_pdu_rx_t *rx_pdu_buf_alloc()
{
    return array_buf_alloc(&rx_pdu_buf);
}

void rx_pdu_buf_retain(network_pdu_rx_t *ptr)
{
    array_buf_retain(&rx_pdu_buf,ptr);
}

void rx_pdu_buf_release(network_pdu_rx_t *ptr)
{
    array_buf_release(&rx_pdu_buf,ptr);
}

void network_rx_process_callback(network_pdu_rx_t **ptr,void *dummy,uint8_t status)
{
    network_pdu_rx_t *pdu = *ptr;
    rx_pdu_buf_release(pdu);
}

void mesh_msg_rx(network_pdu_rx_t *pdu)
{
    network_rx_process_start(pdu);
}

network_pdu_rx_t *network_pdu_alloc_and_fill(uint8_t *data,uint8_t length,uint8_t type,uint8_t from)
{
    network_pdu_rx_t *pdu = rx_pdu_buf_alloc();
    if(pdu)
    {
        memcpy(pdu->src.data,data,length);
        pdu->src.length = length;
        pdu->type = type;
        pdu->from = from;
    }
    return pdu;
}

void adv_mesh_msg_rx(uint8_t *data,uint8_t length,ble_txrx_time_t rx_time,uint8_t rssi)
{
    //LOG_D("%s",__func__);
    if(!get_is_node_can_rx_pkt())
    {
        return;
    }
    network_pdu_rx_t *pdu = network_pdu_alloc_and_fill(data,length,NORMAL_NETWORK_PKT_RX,ADV_BEARER_RX);
    if(pdu == NULL)
    {
        LOG(LOG_LVL_WARN,"adv:no avaiable rx network pdu buf\n");
        return;
    }
    pdu->rx_time = rx_time;
//    LOG(LOG_LVL_WARN,"adv rx tim %d\n",rx_time.time_cnt);
    pdu->rssi = rssi;
    if(adv_mesh_msg_cache_search(pdu))
    {
        //LOG_D("adv_mesh_msg_cach=truee");
        rx_pdu_buf_release(pdu);
    }else
    {
        if(adv_mesh_msg_cache_full())
        {
            network_pdu_rx_t *ptr = adv_mesh_msg_cache_remove_oldest();
            rx_pdu_buf_release(ptr);
        }
        adv_mesh_msg_cache_add(pdu);
        rx_pdu_buf_retain(pdu);
        mesh_msg_rx(pdu);
    }
}

void gatt_mesh_msg_rx(uint8_t *data,uint8_t length,uint8_t type,uint8_t from)
{
    if(!get_is_node_can_rx_pkt())
    {
        //LOG(3, "%s:%d\n", __func__, __LINE__);
        return;
    }
    network_pdu_rx_t *pdu = network_pdu_alloc_and_fill(data,length,type,from);
    if(pdu == NULL)
    {
        LOG(LOG_LVL_WARN,"gatt:no avaiable rx network pdu buf\n");
        return;
    }
    pdu->rx_time =  ble_current_time_get();
    mesh_msg_rx(pdu);
}

void mesh_adv_rx(uint8_t ad_type,uint8_t *data,uint8_t length,ble_txrx_time_t rx_time,uint8_t rssi)
{
    switch(ad_type)
    {
    case MESH_PROVISIONING_AD_TYPE:
        provision_pb_adv_rx(data,length);
        break;
    case MESH_MESSAGE_AD_TYPE:
        adv_mesh_msg_rx(data,length,rx_time,rssi);
        break;
    case MESH_BEACON_AD_TYPE:
        beacon_rx(data,length);
        break;
    }
}



