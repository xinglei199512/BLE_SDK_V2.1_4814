/**
 ****************************************************************************************
 *
 * @file   mesh_gatt_serv_adv.h
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

/**
 ****************************************************************************************
 * @addtogroup MESH_mesh_gatt_serv_adv_API Mesh mesh_gatt_serv_adv API
 * @ingroup MESH_API
 * @brief Mesh mesh_gatt_serv_adv  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_GATT_MESH_GATT_SERV_ADV_H_
#define APP_FREERTOS_MESH_GATT_MESH_GATT_SERV_ADV_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "adv_bearer_tx.h"
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

typedef struct
{
    uint8_t count;
    uint8_t interval_steps;
    uint8_t repeats;
}gatt_service_tx_param_t;
 
typedef struct
{
    mesh_adv_tx_t adv;
    mesh_timer_t rand_timer;
    gatt_service_tx_param_t param;
}gatt_service_adv_tx_t;


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

gatt_service_adv_tx_t *gatt_service_adv_pkt_prepare(uint8_t *data,uint8_t length,uint8_t pkt_type);
void gatt_service_on_adv_start_callback(mesh_adv_tx_t *adv);
void gatt_service_adv_tx_start(gatt_service_adv_tx_t *ptr);

#endif /* APP_FREERTOS_MESH_GATT_MESH_GATT_SERV_ADV_H_ */ 
/// @} MESH_mesh_gatt_serv_adv_API

