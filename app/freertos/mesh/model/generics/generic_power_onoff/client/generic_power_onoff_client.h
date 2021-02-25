#ifndef GENERIC_POWER_ONOFF_CLIENT__H
#define GENERIC_POWER_ONOFF_CLIENT__H
#include <stdint.h>
#include "mesh_model.h"
#include "mesh_env.h"
#include "generic_common.h"
#include "model_servers_events_api.h"
#include "access_rx_process.h"

typedef struct
{
    /** The model base of the model. */
    model_client_base_t model;
}generic_power_onoff_client_t;

#define DEF_GENERIC_POWER_ONOFF_CLIENT_MODEL(name,app_key_max) \
    static generic_power_onoff_client_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_GENERIC_POWER_ONOFF_CLIENT_MODEL(name,app_key_max,element_num,sub_list,sub_list_num) \
    mesh_model_init(&name.model.base,GENERIC_POWER_CLIENT_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);


void generic_power_onoff_status_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

#endif /* GENERIC_ONOFF_CLIENT__H */ 


