#ifndef GENERIC_LEVEL_CLIENT__H
#define GENERIC_LEVEL_CLIENT__H
#include <stdint.h>
#include "mesh_model.h"
//#include "access.h"
#include "mesh_model.h"
#include "mesh_env.h"
#include "generic_common.h"
#include "generic_level_common.h"
#include "model_servers_events_api.h"
#include "access_rx_process.h"

enum GENERIC_LEVEL_CLIENT_EVENT_TYPE
{
    GENERIC_LEVEL_STATUS  = 0x01,
    GENERIC_LEVEL_DEFAULT_STATUS  = 0x02,
    GENERIC_LEVEL_SET,
    GENERIC_LEVEL_SET_UNACKNOWLEDGED,
    GENERIC_DELTA_SET,
    GENERIC_DELTA_SET_UNACKNOWLEDGED,
    GENERIC_MOVE_SET,
    GENERIC_MOVE_SET_UNACKNOWLEDGED,
};

typedef struct
{
    /** The model base of the model. */
    model_client_base_t model;
    user_set_current_value_cb set_current_value_cb;
    user_get_current_state_cb get_current_state_cb;
}generic_level_client_t;

#define DEF_GENERIC_LEVEL_CLIENT_MODEL(name,app_key_max) \
    static generic_level_client_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_GENERIC_LEVEL_CLIENT_MODEL(name,app_key_max,element_num,sub_list,sub_list_num) \
    mesh_model_init(&name.model.base,GENERIC_LEVEL_CLIENT_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num, NULL);\
    mesh_element_init(element_num, &name.model.base);


void generic_level_status_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void generic_level_msg_publish(generic_level_client_t *client, void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode);
#endif /* GENERIC_LEVEL_CLIENT__H */


