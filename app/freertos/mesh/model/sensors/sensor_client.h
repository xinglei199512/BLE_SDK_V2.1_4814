/**
 ****************************************************************************************
 *
 * @file   sensor_client.h
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
 * @addtogroup MESH_sensor_client_API Mesh sensor_client API
 * @ingroup MESH_API
 * @brief Mesh sensor_client  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_SENSORS_SENSOR_CLIENT_H_
#define APP_FREERTOS_MESH_MODEL_SENSORS_SENSOR_CLIENT_H_

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
void sensor_descriptor_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void sensor_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void sensor_colunm_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void sensor_series_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void sensor_cadence_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void sensor_setting_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);



#endif /* APP_FREERTOS_MESH_MODEL_SENSORS_SENSOR_CLIENT_H_ */ 
/// @} MESH_sensor_client_API

