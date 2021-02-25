#include <string.h>
#include "adv_bearer_tx.h"
#include "static_buffer.h"
#include "beacon.h"
#include "queued_async_framework.h"
#include "network_pdu_decrypt.h"
#include "upper_tx_process.h"
#include "mesh_core_api.h"
#include "osapp_task.h"
#include "mesh_sched.h"
#include "stack_mem_cfg.h"
#include "lower_tx_process.h"
#include "log.h"
#include "os_wrapper.h"
#include "mesh_queued_msg.h"
#include "network_tx_process.h"
#include "adv_bearer_tx.h"
#include "provision.h"
#include "mesh_gatt_serv_adv.h"

DEF_ARRAY_BUF(beacon_adv_tx_buf,beacon_adv_tx_t,NETWORK_ADV_TX_BUF_SIZE);

void bearer_adv_tx_msg_send(mesh_adv_tx_t *ptr,bool high_priority)
{
    mesh_sched_adv_tx(ptr,high_priority);
//    LOG(LOG_LVL_WARN,"send : 0x%x\n",ptr);
}

static void beacon_adv_tx_buf_release(beacon_adv_tx_t *ptr)
{
    array_buf_release(&beacon_adv_tx_buf,ptr);
}

void beacon_adv_tx_buf_free(beacon_adv_tx_t *ptr)
{
    beacon_adv_tx_buf_release(ptr);
}

static void unprov_tx_on_adv_start_callback(mesh_adv_tx_t *adv)
{
    beacon_adv_tx_t *ptr = CONTAINER_OF(adv, beacon_adv_tx_t, adv);
    uint16_t interval_ms = ptr->param.interval_steps;

    if(ptr->param.count <= ptr->param.repeats)
    {
        bearer_adv_tx_timer_start(adv, interval_ms);
        ptr->param.count ++;
    }
    else
    {
        mesh_timer_delete(adv->timer);
        beacon_adv_tx_buf_release(ptr);
    }
}

static void beacon_tx_on_adv_start_callback(mesh_adv_tx_t *adv)
{
    
    beacon_adv_tx_t *ptr = CONTAINER_OF(adv,beacon_adv_tx_t, adv);
    beacon_adv_tx_buf_free(ptr);
}

void bearer_on_adv_start_callback(mesh_adv_tx_t *adv)
{
    switch(adv->pkt_type)
    {
        case UNPROV_DEV_BEACON_ADV_PKT:
             unprov_tx_on_adv_start_callback(adv);
        break;
        case SECURE_NETWORK_BEACON_ADV_PKT:
             beacon_tx_on_adv_start_callback(adv);
        break;
        case UPPER_LAYER_ADV_PKT:
        case SEGMENT_ACK_ADV_PKT:
        case RELAY_ADV_PKT:
        case FRIEND_QUEUE_ADV_PKT:
            network_tx_on_adv_start_callback(adv);
        break;
        case GATT_SERVICE_ADV_PKT:
             gatt_service_on_adv_start_callback(adv);
        break;
        case PROVISION_BEARER_ADV_PKT:
            pb_tx_on_adv_start_callback(adv);
        break;
        default:
            BX_ASSERT(0);
        break;
    }
    
}

beacon_adv_tx_t *beacon_adv_tx_buf_alloc(uint8_t pkt_type)
{
    beacon_adv_tx_t *ptr = array_buf_alloc(&beacon_adv_tx_buf);
    ptr->adv.pkt_type = pkt_type;
    return ptr;
}   

beacon_adv_tx_t *beacon_adv_pkt_prepare(uint8_t *data,uint8_t length,uint8_t pkt_type)
{
    beacon_adv_tx_t *adv_tx = beacon_adv_tx_buf_alloc(pkt_type);
    adv_tx->adv.data_length = length;
    memcpy(adv_tx->adv.adv_data,data,length);
    adv_tx->adv.scan_rsp = NULL;
    adv_tx->adv.pkt_type = pkt_type;
    return adv_tx;    
}

bool bearer_adv_tx_timer_cancel(mesh_adv_tx_t *adv)
{
    bool cancelled;
    os_enter_critical();
    cancelled = mesh_timer_active(adv->timer);
    if(cancelled)
    {
        mesh_timer_stop(adv->timer);
    }
    os_exit_critical();
    return cancelled;
}

void bearer_adv_tx_timer_create(char *name, mesh_adv_tx_t *ptr,bool auto_reload,void (*cb)(mesh_timer_t))
{
    ptr->timer = mesh_timer_create(name, 1, auto_reload, ptr, cb);
}

void bearer_adv_tx_timer_start(mesh_adv_tx_t *adv,uint16_t ms)
{
    uint32_t ticks = pdMS_TO_TICKS(ms);
    if(ticks == 0)
    {
        ticks = 1;
    }
    mesh_timer_change_period(adv->timer,ticks);
}

//TODO
void mesh_adv_queue_reset(void)
{
}


