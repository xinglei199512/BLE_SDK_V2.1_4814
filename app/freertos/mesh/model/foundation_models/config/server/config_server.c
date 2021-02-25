
#include "osapp_config.h"
#include "mesh_model.h"
#include "access_rx_process.h"
#include "mesh_core_api.h"
#include "config_server.h"

DEF_CONFIG_SERVER_MODEL(config_server);

uint8_t config_server_get_default_ttl(void)
{
    return config_server.default_ttl;
}

void config_server_set_default_ttl(uint8_t ttl)
{
    config_server.default_ttl = ttl;
}

void regisite_config_server_evt_cb(config_server_evt_cb_t cb)
{
    config_server.config_server_evt_cb = cb;
}

void config_server_evt_act(config_server_evt_type_t type, config_server_evt_param_t * p_param)
{
    if(config_server.config_server_evt_cb)
    {
        config_server.config_server_evt_cb(type,p_param);
    }
}

void config_server_init(void)
{
    INIT_CONFIG_SERVER_MODEL(config_server);
}

config_server_model_t* get_config_server(void)
{
    return &config_server;
}
