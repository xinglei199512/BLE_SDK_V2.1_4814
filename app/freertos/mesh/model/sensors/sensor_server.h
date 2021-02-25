/**
 ****************************************************************************************
 *
 * @file   sensor_server.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2019-02-25 11:06
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_sensor_server_API Mesh sensor_server API
 * @ingroup MESH_API
 * @brief Mesh sensor_server  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_SENSORS_SENSOR_SERVER_H_
#define APP_FREERTOS_MESH_MODEL_SENSORS_SENSOR_SERVER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"

#include "sensor_common.h"
#include "mesh_model.h"
#include "osapp_config.h"
#include "generic_common.h"
#include "model_common.h"
#include "model_servers_events_api.h"
#include "access_rx_process.h"
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
typedef struct
{
    uint16_t property_id;
    uint8_t  fast_cadence_period_divisor:7,
             status_trigger_type: 1;
    uint16_t descriptor;
}sensor_msg_format_t;

typedef struct
{
    model_server_base_t model;
    sensor_msg_format_t msg_format[SENSOR_NUM];
    model_state_bound_field_t *state_bound;
    generic_valid_field_queue_t tid_queue;
    generic_delay_trans_param_t *delay_trans_timer;
    mesh_model_evt_cb_t cb;
    uint8_t server_state;
}sensor_server_t;


void sensor_descriptor_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void sensor_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void sensor_colunm_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void sensor_series_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

#endif /* APP_FREERTOS_MESH_MODEL_SENSORS_SENSOR_SERVER_H_ */ 
/// @} MESH_sensor_server_API

