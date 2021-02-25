#ifndef CONFIG_SERVER_H_
#define CONFIG_SERVER_H_
#include <stddef.h>

#include "mesh_env.h"
#include "mesh_node_base.h"
#include "config_server_key.h"
#include "config_server_feature.h"
#include "config_server_heartbeat.h"
#include "config_server_misc.h"
#include "config_server_pub_sub.h"
#include "config_server_events_api.h"


#define CONFIGURATION_SERVER_MODEL_ID 0x0000
#define DEFAULT_TTL 0x0a

typedef void (*config_server_evt_cb_t)(config_server_evt_type_t type, config_server_evt_param_t * p_param);

typedef struct
{
    model_server_base_t model; 
    uint8_t default_ttl;
    config_server_evt_cb_t config_server_evt_cb;
    heartbeat_publication_state_t heartbeat_publication;
    heartbeat_subscription_state_t heartbeat_subscription;
}config_server_model_t;

#define DEF_CONFIG_SERVER_MODEL(name) \
    static config_server_model_t name;

#define INIT_CONFIG_SERVER_MODEL(name) \
    mesh_model_init(&name.model.base, CONFIGURATION_SERVER_MODEL_ID, true, 0, NULL);\
    model_publish_subscribe_bind(&name.model.base,NULL,NULL,0,NULL);\
    mesh_element_init(0, &name.model.base);\
    name.default_ttl = DEFAULT_TTL;


    
uint8_t config_server_get_default_ttl(void);
void config_server_set_default_ttl(uint8_t ttl);
void regisite_config_server_evt_cb(config_server_evt_cb_t cb);
void config_server_evt_act(config_server_evt_type_t type, config_server_evt_param_t * p_param);
void config_server_init(void);

/**
 ****************************************************************************************
 * @brief   Get the pointer of config server.
 * @return  The pointer of config server.
 ****************************************************************************************
 */
config_server_model_t* get_config_server(void);

    
#endif
