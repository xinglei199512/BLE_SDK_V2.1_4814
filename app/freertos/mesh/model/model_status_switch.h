/**
 ****************************************************************************************
 *
 * @file   model_status_switch.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-11 09:33
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
 * @addtogroup MESH_model_status_switch_API Mesh model_status_switch API
 * @ingroup MESH_API
 * @brief Mesh model_status_switch  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_MODEL_STATUS_SWITCH_H_
#define APP_FREERTOS_MESH_MODEL_MODEL_STATUS_SWITCH_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_config_pro.h"
#include "sdk_mesh_definitions.h"
#include "generic_level_server.h"
#include "generic_onoff_server.h"
#include "light_ctl_server.h"
#include "light_hsl_server.h"
#include "light_lightness_server.h"

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

void status_switch_onoff_to_level(generic_onoff_server_t *onoff_server, generic_level_server_t *level_server, uint16_t lightness_last);
void status_switch_onoff_to_lightness(generic_onoff_server_t *onoff_server, light_lightness_server_t *lightness_server);
void status_switch_onoff_to_ctl(generic_onoff_server_t *onoff_server, light_ctl_server_t *ctl_server, uint16_t lightness_last);
void status_switch_onoff_to_hsl(generic_onoff_server_t *onoff_server, light_hsl_server_t *hsl_server, uint16_t lightness_last);

void status_switch_level_to_onoff(generic_level_server_t *level_server, generic_onoff_server_t *onoff_server);
void status_switch_level_to_lightness(generic_level_server_t *level_server, light_lightness_server_t *lightness_server);
void status_switch_level_to_ctl(generic_level_server_t *level_server, light_ctl_server_t *ctl_server);
void status_switch_level_to_hsl(generic_level_server_t *level_server, light_hsl_server_t *hsl_server);

void status_switch_lightness_to_onoff(light_lightness_server_t *lightness_server, generic_onoff_server_t *onoff_server);
void status_switch_lightness_to_level(light_lightness_server_t *lightness_server, generic_level_server_t *level_server);
void status_switch_lightness_to_ctl(light_lightness_server_t *lightness_server, light_ctl_server_t *ctl_server);
void status_switch_lightness_to_hsl(light_lightness_server_t *lightness_server, light_hsl_server_t *hsl_server);

void status_switch_ctl_to_onoff(light_ctl_server_t *ctl_server, generic_onoff_server_t *onoff_server);
void status_switch_ctl_to_level(light_ctl_server_t *ctl_server, generic_level_server_t *onoff_server);
void status_switch_ctl_to_lightness(light_ctl_server_t *ctl_server, light_lightness_server_t *lightness_server);
void status_switch_ctl_to_hsl(light_ctl_server_t *ctl_server, light_hsl_server_t *hsl_server);


void status_switch_hsl_to_onoff(light_hsl_server_t *hsl_server, generic_onoff_server_t *onoff_server);
void status_switch_hsl_to_level(light_hsl_server_t *hsl_server, generic_level_server_t *level_server);
void status_switch_hsl_to_lightness(light_hsl_server_t *hsl_server, light_lightness_server_t *lightness_server);
void status_switch_hsl_to_ctl(light_hsl_server_t *hsl_server, light_ctl_server_t *ctl_server);
#endif /* APP_FREERTOS_MESH_MODEL_MODEL_STATUS_SWITCH_H_ */ 
/// @} MESH_model_status_switch_API

