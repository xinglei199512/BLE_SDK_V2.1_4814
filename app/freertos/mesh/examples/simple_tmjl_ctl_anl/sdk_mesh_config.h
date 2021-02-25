/**
 ****************************************************************************************
 *
 * @file sdk_mesh_config.h
 *
 * @brief Config BLE Mesh SDK Parameter.
 *
 * Copyright (C) Apollo 2018-2023
 *
 ****************************************************************************************
 */

#ifndef FREERTOS_APP_MESH_CORE_SDK_MESH_CONFIG_H_
#define FREERTOS_APP_MESH_CORE_SDK_MESH_CONFIG_H_

/**
 @defgroup MESH_API
 @{
 @}
 */
/**
 * @addtogroup MESH_SDK_CFG_API
 * @ingroup  MESH_API
 * @brief defines for BLE MESH  sdk config common params
 * @{
 * @}
 */
/**
 ****************************************************************************************
 * @addtogroup MESH_SDK_CFG_SIMPLE
 * @ingroup  MESH_SDK_CFG_API
 * @brief defines for BLE MESH  sdk config common params
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "sdk_mesh_config_pro.h"
#include "mesh_errors.h"
/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/**
 * @brief mesh feature support status.
 */
#define MESH_FEATURE_DISABLED      0
#define MESH_FEATURE_ENABLED       1
#define MESH_FEATURE_NOT_SUPPORT   2

typedef uint8_t mesh_feature_stat_t;

/**
 * @brief mesh roles
 */
typedef enum
{
    /// node is config server.
    MESH_ROLE_CONFIG_SERVER,
    /// node is config client.
    MESH_ROLE_CONFIG_CLIENT,
}mesh_role_t;


/**
 * @brief mesh ADV transmit count and interval
 */
typedef struct
{
    /// the retransmit count of the message.send times = count + 1
    uint8_t count;
    /// the interval between two messages.(unit is 10ms)
    uint8_t interval_steps;
    /// the period between two messages.(unit is 1ms)
    uint16_t period;
}adv_transmit_state_t;

/*Method of configuring network access
 *
 *      bit mask
 *      7      6      5      4      3      2      1     0
 *                                               gatt   adv
 * */
typedef enum
{
    PROVISION_BY_GATT       = 0x01,//1<<0
    PROVISION_BY_ADV        = 0x02,//1<<1
    PROVISION_BY_ADV_GATT   = PROVISION_BY_GATT | PROVISION_BY_ADV,
}mesh_provsion_method_t;
/*
 * DEFINES
 ****************************************************************************************
 */



/**
 @defgroup MESH_FEATURE_SUPPORT mesh feature support status
 @{
 @brief mesh feature support status
*/
#define MESH_SUPPORT_RELAY      MESH_FEATURE_DISABLED       /// support relay feature
#define MESH_SUPPORT_PROXY      MESH_FEATURE_ENABLED        /// support proxy feature
#define MESH_SUPPORT_FRIEND     MESH_FEATURE_NOT_SUPPORT    /// support friend feature
#define MESH_SUPPORT_LOW_POWER  MESH_FEATURE_NOT_SUPPORT    /// support low power feature
/**
 @}MESH_FEATURE_SUPPORT
*/

/* config unprovision device  provision method */
#define MESH_UNPROV_PROVISION_METHOD    PROVISION_BY_ADV//PROVISION_BY_GATT

#define MESH_MODEL_HEALTH_SERVER 1
#define MESH_MODEL_HEALTH_CLIENT 0
#define MESH_MODEL_CONFIG_SERVER 1
#define MESH_MODEL_CONFIG_CLIENT 0
#define MESH_MODEL_GENERIC_ONOFF_SERVER 1
#define MESH_MODEL_GENERIC_ONOFF_CLIENT 0
#define MESH_MODEL_GENERIC_LEVEL_SERVER 1
#define MESH_MODEL_GENERIC_LEVEL_CLIENT 0
#define MESH_MODEL_GENERIC_POWER_ONOFF_SERVER 1
#define MESH_MODEL_GENERIC_POWER_ONOFF_SETUP_SERVER 0
#define MESH_MODEL_GENERIC_POWER_ONOFF_CLIENT 0
#define MESH_MODEL_GENERIC_TRANSITION_SERVER 1
#define MESH_MODEL_GENERIC_TRANSITION_CLIENT 0
#define MESH_MODEL_LIGHT_LIGHTNESS_SERVER 1
#define MESH_MODEL_LIGHT_LIGHTNESS_SETUP_SERVER 1
#define MESH_MODEL_LIGHT_LIGHTNESS_CLIENT 0
#define MESH_MODEL_LIGHT_CTL_SERVER 1
#define MESH_MODEL_LIGHT_CTL_TEMPERATURE_SERVER 0
#define MESH_MODEL_LIGHT_CTL_SETUP_SERVER 0
#define MESH_MODEL_LIGHT_CTL_CLIENT 0
#define MESH_MODEL_LIGHT_HSL_SERVER 1
#define MESH_MODEL_LIGHT_HSL_HUE_SERVER 0
#define MESH_MODEL_LIGHT_HSL_SATURATION_SERVER 0
#define MESH_MODEL_LIGHT_HSL_SETUP_SERVER 0
#define MESH_MODEL_LIGHT_HSL_CLIENT 0
#define MESH_MODEL_TMALL_MODEL_SERVER 1
#define MESH_MODEL_TMALL_MODEL_CLIENT 1 
#define MESH_MODEL_TIME_SERVER 0
#define MESH_MODEL_TIME_SETUP_SERVER 0
#define MESH_MODEL_TIME_CLIENT 0
#define MESH_MODEL_SCENE_SERVER 1
#define MESH_MODEL_SCENE_SETUP_SERVER 0
#define MESH_MODEL_SCENE_CLIENT 0
#define MESH_MODEL_SCHEDULER_SERVER 0
#define MESH_MODEL_SCHEDULER_SETUP_SERVER 0
#define MESH_MODEL_SCHEDULER_CLIENT 0



#endif /* FREERTOS_APP_MESH_CORE_SDK_MESH_CONFIG_H_ */
/// @} MESH_SDK_CFG_SIMPLE
