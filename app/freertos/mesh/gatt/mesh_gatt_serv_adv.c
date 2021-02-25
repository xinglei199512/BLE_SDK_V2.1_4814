/**
 ****************************************************************************************
 *
 * @file   mesh_gatt_serv_adv.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-29 17:05
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) BlueX Microelectronics 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "mesh_gatt_serv_adv.h"
#include "stack_mem_cfg.h"
#include "adv_bearer_tx.h"
#include "mesh_core_api.h"
#include "mesh_queued_msg.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
DEF_ARRAY_BUF(gatt_service_adv_tx_buf,gatt_service_adv_tx_t,GATT_SERVICE_ADV_TX_BUF_SIZE);

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */



void gatt_service_adv_tx_buf_free(gatt_service_adv_tx_t *ptr)
{
    mesh_timer_delete(ptr->adv.timer);
    ptr->adv.timer = NULL;
    array_buf_release(&gatt_service_adv_tx_buf,ptr);
}


static void gatt_service_tx_msg_send(gatt_service_adv_tx_t *ptr) 
{
    bearer_adv_tx_msg_send(&ptr->adv , false);
    ptr->param.count++;
}

static void gatt_service_tx_timer_expire_handler(gatt_service_adv_tx_t *ptr)
{
    gatt_service_tx_msg_send(ptr);
}

void gatt_service_adv_tx_timer_callback(mesh_timer_t xTimer)
{
    mesh_adv_tx_t *adv = pvTimerGetTimerID(xTimer);
    gatt_service_adv_tx_t *ptr = CONTAINER_OF(adv, gatt_service_adv_tx_t, adv);
    //LOG(3,"gatt_service_adv_tx_timer_callback\n");
    mesh_queued_msg_send((void (*)(void *))gatt_service_tx_timer_expire_handler, ptr);
}

gatt_service_adv_tx_t *gatt_service_adv_tx_buf_alloc(uint8_t pkt_type)
{
    gatt_service_adv_tx_t *ptr = array_buf_alloc(&gatt_service_adv_tx_buf);
    bearer_adv_tx_timer_create("gatt_adv_tx", &ptr->adv, false, gatt_service_adv_tx_timer_callback);
    ptr->adv.pkt_type = pkt_type;
    BX_ASSERT(NULL!=ptr);

    mesh_core_params_t param_proxy_services_transmit;
    mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_PROXY_SERVICES_TRANSMIT   , &param_proxy_services_transmit);
    ptr->param.count = 0;
    ptr->param.interval_steps = (param_proxy_services_transmit.proxy_services_transmit.interval_steps + 1)* 10;
    ptr->param.repeats = param_proxy_services_transmit.proxy_services_transmit.count;
    return ptr;
}

gatt_service_adv_tx_t *gatt_service_adv_pkt_prepare(uint8_t *data,uint8_t length,uint8_t pkt_type)
{
    gatt_service_adv_tx_t *adv_tx = gatt_service_adv_tx_buf_alloc(pkt_type);
    adv_tx->adv.data_length = length;
    memcpy(adv_tx->adv.adv_data,data,length);
    adv_tx->adv.scan_rsp = NULL;
    adv_tx->adv.pkt_type = pkt_type;
    return adv_tx;    
}

void gatt_service_on_adv_start_callback(mesh_adv_tx_t *adv)
{
    gatt_service_adv_tx_t *ptr = CONTAINER_OF(adv, gatt_service_adv_tx_t, adv);
    uint16_t interval_ms = ptr->param.interval_steps;

    if(ptr->param.count <= ptr->param.repeats)
    {
        bearer_adv_tx_timer_start(adv, interval_ms);
    }
    else
    {
       gatt_service_adv_tx_buf_free(ptr);
    }

}

void gatt_service_adv_tx_start(gatt_service_adv_tx_t *ptr)
{
    bearer_adv_tx_timer_start(&ptr->adv, 1);
}

