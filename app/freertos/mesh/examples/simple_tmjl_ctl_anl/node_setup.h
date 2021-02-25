/**
 ****************************************************************************************
 *
 * @file   node_setup.h
 *
 * @brief  .
 *
 * @author  liuzy
 * @date    2018-09-25 17:20
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_mesh_app_API Mesh mesh_app API
 * @ingroup MESH_API
 * @brief Mesh mesh_app  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef FREERTOS_APP_MESH_EXAMPLES_SIMPLE_GENERIC_ONOFF_SERVER_NODE_SETUP_H_
#define FREERTOS_APP_MESH_EXAMPLES_SIMPLE_GENERIC_ONOFF_SERVER_NODE_SETUP_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <model_common.h>

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
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void mesh_app_server_init(void);
//init user models
void mesh_node_setup(void);
void user_model_parameter_init(void);
void model_status_publish(void);
void user_reset_timer_callback(mesh_timer_t xTimer);
void tmall_test_time(void);
//void user_generic_onoff_status_publish(void);
void user_generic_onoff_status_publish(uint8_t present_onoff,uint8_t target_onoff);
void tmall_test_indication_onoff(bool onoff);
void tmall_test_indication_key0(void);
void user_scan_stop_callback(void);

typedef void (*client_send)(uint32_t opcode, uint16_t dst_addr, uint16_t value, uint8_t use_delay, uint8_t trans_time, uint8_t delay, uint8_t *data);

#endif /* FREERTOS_APP_MESH_EXAMPLES_SIMPLE_GENERIC_ONOFF_SERVER_NODE_SETUP_H_ */ 
/// @} MESH_mesh_app_API

