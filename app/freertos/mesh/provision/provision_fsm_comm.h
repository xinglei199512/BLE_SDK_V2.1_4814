/**
 ****************************************************************************************
 *
 * @file   provision_fsm_comm.h
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-27 16:55
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) BlueX Microelectronics 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_provision_fsm_comm_API Mesh provision_fsm_comm API
 * @ingroup MESH_API
 * @brief Mesh provision_fsm_comm  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_PROVISION_PROVISION_FSM_COMM_H_
#define APP_FREERTOS_MESH_PROVISION_PROVISION_FSM_COMM_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "provision_comm.h"
#include "provision_api.h"
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

typedef union
{
    prov_pkt_in_t pkt;
    prov_link_close_t close_param;
} prov_fsm_params_t;
 
typedef void (*prov_fsm_pkt_send)(void * instance, uint8_t* data, uint8_t length,void (*cb)(void *tcb, Tx_Reason reason));
typedef int32_t (* prov_fsm_compare)(void * cookie, void * p_item);
typedef void (*prov_fsm_pkt_cancel)(void * instance,void (*cb)(void *tcb, Tx_Reason reason));
typedef void (*prov_fsm_prepare_link_close)(void * cookie,uint8_t reason,void (*cb)(void *tcb, Tx_Reason reason));
typedef void (*prov_fsm_link_close)(void * cookie,uint8_t reason);
typedef struct
{
    prov_fsm_state_t state;
    uint8_t timeout_flag;
    uint8_t used;
    uint8_t type;
    mesh_provsion_role_t role;
    mesh_timer_t  state_timeout_timer;
    void * cookie;
    prov_fsm_pkt_send pkt_send;
    prov_fsm_compare item_compare;
    prov_fsm_pkt_cancel cancel_tx;
    prov_fsm_prepare_link_close pre_close_func;
    prov_fsm_link_close close_func;

}prov_fsm_tcb_t;

  
typedef void (*prov_fsm_handler_t)(prov_fsm_tcb_t *tcb, prov_fsm_params_t *param);

typedef const struct
{
    prov_fsm_state_t   state;
    prov_fsm_evt_t     evt;
    prov_fsm_state_t   next_state;
    prov_fsm_handler_t fsm_handler;
}prov_fsm_table_t;

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void prov_fsm_tx_cancel(prov_fsm_tcb_t *tcb);
void prov_fsm_tx_pkt(prov_fsm_tcb_t *tcb, uint8_t *data, uint8_t length);
void prov_fsm_evt_handle(prov_fsm_tcb_t *tcb,prov_fsm_evt_t evt,prov_fsm_params_t *param);
void provision_fsm_entry(prov_fsm_tcb_t * tcb, uint8_t * pkt, uint16_t length);
void prov_fsm_set_stage_timeout(prov_fsm_tcb_t *tcb);
void prov_fsm_timeout_handle(prov_fsm_tcb_t *tcb);
prov_fsm_tcb_t * prov_fsm_create_instance(mesh_provsion_role_t  role);
void * prov_fsm_get_adv_protocol_instance(void * item);
void * prov_fsm_create_adv_protocol_instance(    void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx);
void * prov_fsm_get_gatt_protocol_instance(void * item);
void * prov_fsm_create_gatt_protocol_instance(    void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx);
void * prov_get_cookie(void * env);
void prov_fsm_ack_finish(prov_fsm_tcb_t *tcb,void * item);
void  prov_fsm_adv_close_link(void * item,uint8_t reason);
void  prov_fsm_gatt_close_link(void * item);
void prov_fsm_tx_fail (prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param);
void prov_fsm_prepare_close_link (prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param);
void prov_fsm_ack_tx_cancle (prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param);
void prov_fsm_close_protocol_instance(    prov_fsm_tcb_t *tcb);

#endif /* APP_FREERTOS_MESH_PROVISION_PROVISION_FSM_COMM_H_ */ 
/// @} MESH_provision_fsm_comm_API

