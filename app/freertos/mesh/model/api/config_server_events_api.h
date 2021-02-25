/*
 * config_server_events_api.h
 *
 *  Created on: 2018-9-10
 *      Author: jiachuang
 */

#ifndef FREERTOS_APP_MESH_MODEL_CONFIG_SERVER_EVENTS_API_H_
#define FREERTOS_APP_MESH_MODEL_CONFIG_SERVER_EVENTS_API_H_
/**
 ****************************************************************************************
 * @addtogroup MESH_MODEL_CONFIG_SERVER_EVENTS
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
#include "access_rx_process.h"


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/** Configuration server event type. */
typedef enum
{
    CONFIG_SERVER_EVT_APPKEY_ADD,
    CONFIG_SERVER_EVT_APPKEY_UPDATE,
    CONFIG_SERVER_EVT_MODEL_PUBLICATION_SET,
    CONFIG_SERVER_EVT_APPKEY_DELETE,
    CONFIG_SERVER_EVT_BEACON_SET,
    CONFIG_SERVER_EVT_DEFAULT_TTL_SET,
    CONFIG_SERVER_EVT_FRIEND_SET,
    CONFIG_SERVER_EVT_GATT_PROXY_SET,
    CONFIG_SERVER_EVT_KEY_REFRESH_PHASE_SET,
    CONFIG_SERVER_EVT_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET,
    CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_ADD,
    CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_DELETE,
    CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_DELETE_ALL,
    CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_OVERWRITE,
    CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD,
    CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE,
    CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE,
    CONFIG_SERVER_EVT_NETWORK_TRANSMIT_SET,
    CONFIG_SERVER_EVT_RELAY_SET,
    CONFIG_SERVER_EVT_LOW_POWER_NODE_POLLTIMEOUT_SET,
    CONFIG_SERVER_EVT_HEARTBEAT_PUBLICATION_SET,
    CONFIG_SERVER_EVT_HEARTBEAT_SUBSCRIPTION_SET,
    CONFIG_SERVER_EVT_MODEL_APP_BIND,
    CONFIG_SERVER_EVT_MODEL_APP_UNBIND,
    CONFIG_SERVER_EVT_NETKEY_ADD,
    CONFIG_SERVER_EVT_NETKEY_DELETE,
    CONFIG_SERVER_EVT_NETKEY_UPDATE,
    CONFIG_SERVER_EVT_NODE_IDENTITY_SET,
    CONFIG_SERVER_EVT_NODE_RESET,
}config_server_evt_type_t;






/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

typedef struct
{
    uint16_t features;
    uint16_t netkey_idx;
    uint16_t dst_addr;
    uint16_t count;
    uint16_t period;
    uint8_t ttl;
}heartbeat_publication_state_t;

typedef struct
{
    uint16_t src_addr;
    uint16_t dst_addr;
    uint16_t count;
    uint16_t period;
    uint8_t min_hops;
    uint8_t max_hops;
}heartbeat_subscription_state_t;



typedef struct
{
    dm_appkey_handle_t appkey_handle;
} config_server_evt_appkey_add_t;

typedef struct
{
    dm_appkey_handle_t appkey_handle;
} config_server_evt_appkey_update_t;

typedef struct
{
    model_base_t *model;
} config_server_evt_publication_set_t;

typedef struct
{
    dm_appkey_handle_t appkey_handle;
} config_server_evt_appkey_delete_t;

typedef struct
{
    bool beacon_state;
} config_server_evt_beacon_set_t;

typedef struct
{
    uint8_t default_ttl;
} config_server_evt_default_ttl_set_t;

typedef struct
{
    mesh_key_refresh_phase_t kr_phase;
} config_server_evt_key_refresh_phase_set_t;

typedef struct
{
    model_base_t *model;
    uint8_t label_uuid[LABEL_UUID_SIZE];
} config_server_evt_publication_virt_addr_add_t;


typedef struct
{
    model_base_t *model;
    uint16_t address;
} config_server_evt_subscription_add_t;

typedef struct
{
    model_base_t *model;
    uint16_t address;
} config_server_evt_subscription_delete_t;

typedef struct
{
    model_base_t *model;
} config_server_evt_subscription_delete_all_t;

typedef struct
{
    model_base_t *model;
    uint16_t address;
} config_server_evt_subscription_overwrite_t;

typedef struct
{
    model_base_t *model;
    uint8_t label_uuid[LABEL_UUID_SIZE];
} config_server_evt_subscription_virt_addr_add_t;

typedef struct
{
    model_base_t *model;
    uint8_t label_uuid[LABEL_UUID_SIZE];
} config_server_evt_subscription_virt_addr_del_t;

typedef struct
{
    model_base_t *model;
    uint8_t label_uuid[LABEL_UUID_SIZE];
} config_server_evt_subscription_virt_addr_overwrite_t;


typedef struct
{
    uint8_t retransmit_count;
    uint8_t interval_steps;
} config_server_evt_network_transmit_set_t;

typedef struct
{
    bool enabled;
}config_server_evt_friend_set_t;

typedef struct
{
    bool enabled;
} config_server_evt_gatt_proxy_set_t;

typedef struct
{
    uint8_t  time;
}config_server_evt_lowpower_polltimeout_set_t; 

typedef struct
{
    uint8_t enabled;
    uint8_t retransmit_count;
    uint8_t interval_steps;
} config_server_evt_relay_set_t;

typedef struct
{
    const heartbeat_publication_state_t * p_publication_state;
} config_server_evt_heartbeat_publication_set_t;

typedef struct
{
    const heartbeat_subscription_state_t * p_subscription_state;
} config_server_evt_heartbeat_subscription_set_t;

typedef struct
{
    model_base_t *model;
    dm_appkey_handle_t appkey_handle;
} config_server_evt_model_app_bind_t;

typedef struct
{
    model_base_t *model;
    dm_appkey_handle_t appkey_handle;
} config_server_evt_model_app_unbind_t;

typedef struct
{
    dm_netkey_handle_t netkey_handle;
} config_server_evt_netkey_add_t;

typedef struct
{
    dm_netkey_handle_t netkey_handle;
} config_server_evt_netkey_delete_t;

typedef struct
{
    dm_netkey_handle_t netkey_handle;
} config_server_evt_netkey_update_t;

typedef struct
{
    dm_netkey_handle_t netkey_handle;
} config_server_evt_node_identity_set_t;

typedef struct
{
    uint32_t  reason;
} config_server_evt_node_reset_t;


typedef union
{
   config_server_evt_appkey_add_t   appkey_add;
   config_server_evt_appkey_update_t   appkey_update;
   config_server_evt_publication_set_t model_publication_set;
   config_server_evt_appkey_delete_t appkey_delete;
   config_server_evt_beacon_set_t beacon_set;
   config_server_evt_default_ttl_set_t default_ttl_set;
   config_server_evt_friend_set_t  friend_set;
   config_server_evt_gatt_proxy_set_t  gatt_proxy_set;
   config_server_evt_key_refresh_phase_set_t   key_refresh_phase_set;
   config_server_evt_publication_virt_addr_add_t model_publication_virt_addr_add;
   config_server_evt_subscription_add_t  model_subscription_add;
   config_server_evt_subscription_delete_t model_subscription_delete;
   config_server_evt_subscription_delete_all_t  model_subscription_delete_all;
   config_server_evt_subscription_overwrite_t  model_subscription_overwrite;
   config_server_evt_subscription_virt_addr_add_t  model_subscription_virt_addr_add;
   config_server_evt_subscription_virt_addr_del_t  model_subscription_virt_addr_delete;
   config_server_evt_subscription_virt_addr_overwrite_t  model_subscription_virt_addr_overwrite;
   config_server_evt_network_transmit_set_t  network_transmit_set;
   config_server_evt_relay_set_t   relay_set;
   config_server_evt_lowpower_polltimeout_set_t node_low_power_poll_timeout_set;
   config_server_evt_heartbeat_publication_set_t heartbeat_publication_set;
   config_server_evt_heartbeat_subscription_set_t heartbeat_subscription_set;
   config_server_evt_model_app_bind_t model_app_bind;
   config_server_evt_model_app_unbind_t model_app_unbind;
   config_server_evt_netkey_add_t netkey_add;
   config_server_evt_netkey_delete_t netkey_delete;
   config_server_evt_netkey_update_t netkey_update;
   config_server_evt_node_identity_set_t node_identity_set;
   config_server_evt_node_reset_t  node_reset;
} config_server_evt_param_t;



/**
 * @brief     Config server event callback type.
 * @param[in] p_evt Event pointer from the configuration server.
 */
 
typedef void (*config_server_evt_cb_t)(config_server_evt_type_t type, config_server_evt_param_t * p_param);


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 * @brief     set config server event callback function.
 * @param[in] cb callback of config server.
 */
void regisite_config_server_evt_cb(config_server_evt_cb_t cb);



/// @} MESH_MODEL_CONFIG_SERVER_EVENTS
#endif /* FREERTOS_APP_MESH_MODEL_CONFIG_SERVER_EVENTS_API_H_ */
