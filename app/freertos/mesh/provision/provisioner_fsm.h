/**
 ****************************************************************************************
 *
 * @file   provisioner_fsm.h
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-16 18:38
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
 * @addtogroup MESH_provisioner_fsm_API Mesh provisioner_fsm API
 * @ingroup MESH_API
 * @brief Mesh provisioner_fsm  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_PROVISION_PROVISIONER_FSM_H_
#define APP_FREERTOS_MESH_PROVISION_PROVISIONER_FSM_H_

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
#define    GET_CLIENT_TCB_PTR(ptr)   CONTAINER_OF(ptr,prov_client_fsm_tcb_t,fsm_tcb)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */





/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


 
typedef struct
{
   prov_fsm_tcb_t fsm_tcb;
   uint8_t dev_uuid[MESH_DEVICE_UUID_LENGTH];
   uint8_t devkey[MESH_KEY_LENGTH];
   uint8_t attention_duration;
   uint16_t  addr;
   provision_capabilities_t provision_capabilities;
   provision_authentication_t  provision_authentication;
   provision_confirmation_t    confirmation; 
   provision_random_t          random;
   provision_start_t start;
   provision_key_t   provision_key;
   session_info_t               session_info;
   provision_encrypted_data_t   provision_encrypted_data;
   uint8_t confirmation_input[CONFIRMATION_INPUTS_BUF_SIZE];
}prov_client_fsm_tcb_t;


typedef struct
{
    prov_client_fsm_tcb_t * tcb;
}prov_client_tcb_buff_t;


typedef struct
{
    mesh_provsion_method_t method;
    uint8_t attention_duration;
    mesh_beacon_t beacon;
}provision_prov_t;

typedef struct
{
    mesh_prov_evt_cb_t evt;
    provision_prov_t   val;
    provision_data_t   provisioner_resources;
    provisioner_key_t  provisioner_key;
    uint8_t inited;
} provisioner_t;
/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
prov_fsm_tcb_t * prov_client_create_instance(void);
void * prov_client_create_adv_protocol_instance(    void * cookie, prov_fsm_pkt_send pkt_send,prov_fsm_compare item_compare,prov_fsm_pkt_cancel cancel_tx);
void prov_client_free_protocol_instance(prov_fsm_tcb_t *tcb);
void * prov_client_get_adv_protocol_instance(void * item);
void prov_client_get_handler_table(prov_fsm_tcb_t *fsm_tcb, prov_fsm_table_t **ppFsm_table, uint8_t *p_table_size);
prov_fsm_tcb_t * client_search_dev_tcb(uint8_t* dev_uuid);
prov_fsm_tcb_t * prov_client_find_dev(uint8_t  *dev_uuid);
void prov_client_start_provision(prov_fsm_tcb_t * fsm_tcb);
void provisioner_input_auth(prov_fsm_tcb_t *fsm_tcb, uint8_t * auth_data);
void provisioner_input_start(prov_fsm_tcb_t *fsm_tcb, uint8_t * start_data);
void provisioner_input_oob_key(prov_fsm_tcb_t *fsm_tcb, uint8_t * key_data);
void prov_client_close_protocol_instance(    void * item,uint8_t reason);


#endif /* APP_FREERTOS_MESH_PROVISION_PROVISIONER_FSM_H_ */ 
/// @} MESH_provisioner_fsm_API

