#ifndef GENERIC_ONOFF_CLIENT_H_
#define GENERIC_ONOFF_CLIENT_H_
#include "mesh_model.h"
#include "generic_common.h"
#include "access_rx_process.h"

#define GENERIC_ONOFF_CLIENT_MODEL_ID 0x1001

enum GENERIC_ONOFF_CLIENT_EVENT_TYPE
{
    GENERIC_ONOFF_STATUS  = 0x01,
    GENERIC_ONOFF_DEFAULT_STATUS  = 0x02,
    GENERIC_ONOFF_SET,
    GENERIC_ONOFF_SET_UNACKNOWLEGED,
};

typedef struct
{
    /** The model base of the model. */
    model_client_base_t model;
    user_set_current_value_cb set_current_value_cb;
    user_get_current_state_cb get_current_state_cb;
}generic_onoff_client_t;

#define DEF_GENERIC_ONOFF_CLIENT_MODEL(name,app_key_max) \
    static generic_onoff_client_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_GENERIC_ONOFF_CLIENT_MODEL(name,app_key_max,element_num,sub_list,sub_list_num) \
    mesh_model_init(&name.model.base,GENERIC_ONOFF_CLIENT_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num, NULL);\
    mesh_element_init(element_num, &name.model.base);

void generic_onoff_status_rx(mesh_elmt_t *elmt,model_base_t *model, access_pdu_rx_t *pdu);

void generic_onoff_msg_publish(generic_onoff_client_t *client,void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode);

#endif


