#ifndef GENERIC_POWER_ONOFF_SERVER__H
#define GENERIC_POWER_ONOFF_SERVER__H
#include <stdint.h>
#include "mesh_model.h"
#include "osapp_config.h"
#include "model_common.h"
#include "model_servers_events_api.h"
#include "generic_power_onoff_common.h"
#include "generic_common.h"
#include "access_rx_process.h"

typedef struct
{
    uint8_t onpowerup;
}generic_power_onoff_msg_format_t;

typedef struct
{
    model_server_base_t model;
    model_state_bound_field_t *state_bound;
    generic_power_onoff_msg_format_t *msg_format;    
    mesh_model_evt_cb_t cb;
}generic_power_onoff_server_t;

typedef struct
{
    model_server_base_t model;
    generic_power_onoff_msg_format_t *msg_format;    
    mesh_model_evt_cb_t cb;
}generic_power_onoff_setup_server_t;

#define DEF_GENERIC_POWER_ONOFF_SERVER_MODEL(name,app_key_max) \
    static generic_power_onoff_server_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_GENERIC_POWER_ONOFF_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base, GENERIC_POWER_ONOFF_SERVER_MODEL_ID, true, app_key_max, name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,(model_publish_timeout_cb_t)generic_onpowerup_status_publish);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

#define DEF_GENERIC_POWER_ONOFF_SETUP_SERVER_MODEL(name,app_key_max) \
    static generic_power_onoff_setup_server_t name;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_GENERIC_POWER_ONOFF_SETUP_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base, GENERIC_POWER_ONOFF_SETUP_SERVER_MODEL_ID, true, app_key_max, name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,NULL,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

void generic_onpowerup_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_onpowerup_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_onpowerup_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_onpowerup_status_publish(generic_power_onoff_server_t *server);

#endif /* GENERIC_ONOFF_SERVER__H */


