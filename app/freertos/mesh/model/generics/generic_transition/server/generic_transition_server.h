#ifndef GENERIC_TRANSITION_SERVER__H
#define GENERIC_TRANSITION_SERVER__H
#include <stdint.h>
#include "mesh_model.h"
#include "osapp_config.h"
#include "model_common.h"
#include "model_servers_events_api.h"
#include "generic_transition_common.h"
#include "generic_common.h"
#include "access_rx_process.h"

typedef struct
{
    model_server_base_t model;
    mesh_model_evt_cb_t cb;
    uint8_t trans_time;
}generic_transition_server_t;

#define DEF_GENERIC_TRANSITION_SERVER_MODEL(name,app_key_max) \
      static generic_transition_server_t name;\
      static model_publish_state_t name##_publish_state;\
      static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_GENERIC_TRANSITION_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,GENERIC_DEFAULT_TRANSITION_TIME_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,(model_publish_timeout_cb_t)generic_transition_status_publish);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

void generic_default_transition_time_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_default_transition_time_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_default_transition_time_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

//user interface
uint8_t user_generic_default_transition_time_get(generic_transition_server_t *server);
void generic_transition_status_publish(generic_transition_server_t *server);

#endif /* GENERIC_TRANSITION_SERVER__H */


