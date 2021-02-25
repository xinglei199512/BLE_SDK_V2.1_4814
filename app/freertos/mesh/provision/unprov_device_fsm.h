/**
 ****************************************************************************************
 *
 * @file   unprov_device_fsm.h
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-16 18:37
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
 * @addtogroup MESH_unprov_device_fsm_API Mesh unprov_device_fsm API
 * @ingroup MESH_API
 * @brief Mesh unprov_device_fsm  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_PROVISION_UNPROV_DEVICE_FSM_H_
#define APP_FREERTOS_MESH_PROVISION_UNPROV_DEVICE_FSM_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "provision_comm.h"
#include "provision_fsm_comm.h"
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
 
typedef enum
{
    BLE_MESH_CLOSE_EVT_PROVISIONING_DONE,
    BLE_MESH_CLOSE_EVT_KEY_DONE,
} mesh_prov_server_close_evt_t;


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */



typedef struct
{
    prov_fsm_tcb_t fsm_tcb;
    prov_fsm_state_t state;
    provision_invite_t  invite;
    provision_key_t   provision_key;
    provision_start_t  select_capabilites;
    provision_authentication_t  provision_authentication;
    provision_confirmation_t    confirmation; 
    provision_random_t          random;
    provision_encrypted_data_t   provision_encrypted_data;
    provision_data_t             provision_data;
    session_info_t               session_info;
    uint8_t static_auth_value[AUTHVALUE_LEN]; 
    uint8_t devkey[MESH_KEY_LENGTH];                //device key when provision complete.
}prov_server_fsm_tcb_t;


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
#define    GET_SERVER_TCB_PTR(ptr)   CONTAINER_OF(ptr,prov_server_fsm_tcb_t,fsm_tcb)


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void prov_serv_free_instance(prov_server_fsm_tcb_t *tcb);
void * prov_serv_create_adv_protocol_instance(    void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx);
void * prov_serv_get_adv_protocol_instance(void * item);
void * prov_serv_create_gatt_protocol_instance(    void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx);
void   prov_serv_free_gatt_protocol_instance(    void * cookie);
void * prov_serv_get_gatt_protocol_instance(void * item);
prov_fsm_tcb_t * prov_serv_create_instance(void);

void prov_server_get_handler_table(prov_fsm_tcb_t *tcb, prov_fsm_table_t **ppFsm_table, uint8_t *p_table_size);
void prov_server_free_instance(prov_fsm_tcb_t *tcb);
void prov_serv_close_protocol_instance(    void * item,uint8_t reason);
void prov_serv_gatt_close_protocol_instance(void * cookie);
void prov_server_changeto_idle(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param);
void prov_server_provision_finish(prov_fsm_tcb_t *fsm_tcb, prov_fsm_params_t *param);

void prov_server_update(mesh_prov_server_close_evt_t evt);

#endif /* APP_FREERTOS_MESH_PROVISION_UNPROV_DEVICE_FSM_H_ */ 
/// @} MESH_unprov_device_fsm_API

