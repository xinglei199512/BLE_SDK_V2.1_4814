/*
 * config_client_events_api.h
 *
 *  Created on: 2018-9-10
 *      Author: jiachuang
 */

#ifndef FREERTOS_APP_MESH_MODEL_CONFIG_CLIENT_EVENTS_API_H_
#define FREERTOS_APP_MESH_MODEL_CONFIG_CLIENT_EVENTS_API_H_
/**
 ****************************************************************************************
 * @addtogroup MESH_MODEL_CONFIG_CLIENT_EVENTS
 * @ingroup  MESH_MODEL_API
 * @brief defines for BLE MESH MODEL API
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_model.h"


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/** Configuration client event type. */
typedef enum
{
    CONFIG_CLIENT_EVT_BEACON_STATUS                      ,
    CONFIG_CLIENT_EVT_COMPOSITION_DATA_STATUS            ,
    CONFIG_CLIENT_EVT_DEFAULT_TTL_STATUS                 ,
    CONFIG_CLIENT_EVT_GATT_PROXY_STATUS                  ,
    CONFIG_CLIENT_EVT_RELAY_STATUS                       ,
    CONFIG_CLIENT_EVT_MODEL_PUBLICATION_STATUS           ,
    CONFIG_CLIENT_EVT_MODEL_SUBSCRIPTION_STATUS          ,
    CONFIG_CLIENT_EVT_NETKEY_STATUS                      ,
    CONFIG_CLIENT_EVT_APPKEY_STATUS                      ,
    CONFIG_CLIENT_EVT_NODE_IDENTITY_STATUS               ,
    CONFIG_CLIENT_EVT_MODEL_APP_STATUS                   ,
    CONFIG_CLIENT_EVT_NODE_RESET_STATUS                  ,
    CONFIG_CLIENT_EVT_FRIEND_STATUS                      ,
    CONFIG_CLIENT_EVT_KEY_REFRESH_PHASE_STATUS           ,
    CONFIG_CLIENT_EVT_HEARTBEAT_PUBLICATION_STATUS       ,
    CONFIG_CLIENT_EVT_LOW_POWER_NODE_POLLTIMEOUT_STATUS  ,
    CONFIG_CLIENT_EVT_NETWORK_TRANSMIT_STATUS            ,
}config_client_evt_type_t;


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


typedef struct
{
    uint8_t status;
} config_client_evt_beacon_status_t;

typedef struct
{
    uint32_t page;
} config_client_evt_composition_data_status_t;

typedef struct
{
    uint8_t default_ttl;
} config_client_evt_default_ttl_status_t;

typedef struct
{
    uint8_t status;
}config_client_evt_gatt_proxy_status_t;

typedef struct
{
    uint8_t enabled;
    uint8_t retransmit_count;
    uint8_t interval_steps;
}config_client_evt_relay_status_t;


typedef struct
{
    uint8_t status;
}config_client_evt_model_publication_status_t;

typedef struct
{
    uint8_t status;
}config_client_evt_model_subscription_status_t;

typedef struct
{
    uint8_t status;
    dm_netkey_handle_t netkey_handle;
}config_client_evt_netkey_status_t;

typedef struct
{
    uint8_t status;
    dm_netkey_handle_t netkey_handle;
    dm_appkey_handle_t appkey_handle;
}config_client_evt_appkey_status_t;

 typedef struct
{
    dm_appkey_handle_t appkey_handle;
    uint8_t            node_identity_state;
}config_client_evt_node_identity_status_t;

typedef struct
{   
    uint8_t status;
    uint16_t addr;
    dm_appkey_handle_t appkey_handle;
    bool sig_model;
    union
    {
      uint16_t modelid_16;
      uint32_t modelid_32;
    }model_id;
}config_client_evt_model_app_status_t;


typedef struct
{
    uint8_t reason;
}config_client_evt_node_reset_status_t;

typedef struct
{
    uint8_t status;
}config_client_evt_friend_status_t;


typedef struct
{
    uint8_t status;
    dm_netkey_handle_t netkey_handle;
    mesh_key_refresh_phase_t phase;
}config_client_evt_key_refresh_phase_status_t;

typedef struct
{
    uint8_t status;
}config_client_evt_hearbeat_publication_status_t;
typedef struct
{
    uint8_t status;
    uint16_t source;
    uint16_t destination;
    uint8_t periodlog;
    uint8_t countlog;
    uint8_t minhops;
    uint8_t maxhops;
}config_client_evt_hearbeat_subscription_status_t;

typedef struct
{
    uint8_t time;
}config_client_evt_lowpower_pull_timeout_status_t;


typedef struct
{
    uint8_t retransmit_count;
    uint8_t interval_steps;
}config_client_evt_network_transmit_status_t;


typedef union
{
    config_client_evt_beacon_status_t beacon_status;
    config_client_evt_composition_data_status_t composition_data_status;
    config_client_evt_default_ttl_status_t default_ttl_status;
    config_client_evt_gatt_proxy_status_t gatt_proxy_status;
    config_client_evt_relay_status_t relay_status;
    config_client_evt_model_publication_status_t model_publication_status;
    config_client_evt_model_subscription_status_t model_subscription_status;
    config_client_evt_netkey_status_t netkey_status;
    config_client_evt_appkey_status_t appkey_status;
    config_client_evt_node_identity_status_t node_identity_status;
    config_client_evt_model_app_status_t model_app_status;
    config_client_evt_node_reset_status_t node_reset_status;
    config_client_evt_friend_status_t friend_status;
    config_client_evt_key_refresh_phase_status_t key_refresh_phase_status;
    config_client_evt_hearbeat_publication_status_t hearbeat_publication_status;
    config_client_evt_hearbeat_subscription_status_t hearbeat_subscription_status;
    config_client_evt_lowpower_pull_timeout_status_t lowpower_pull_timeout_status;
    config_client_evt_network_transmit_status_t network_transmit_status;
} config_client_evt_param_t;





/**
 * @brief     Config client event callback type.
 * @param[in] p_evt Event pointer from the configuration client.
 */
typedef void (*config_client_evt_cb_t)(config_client_evt_type_t type, config_client_evt_param_t * p_param);


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 * @brief     set config client event callback function.
 * @param[in] cb callback of config client.
 */
void regisite_config_client_evt_cb(config_client_evt_cb_t cb);



/// @} MESH_MODEL_CONFIG_CLIENT_EVENTS
#endif /* FREERTOS_APP_MESH_MODEL_CONFIG_CLIENT_EVENTS_API_H_ */
