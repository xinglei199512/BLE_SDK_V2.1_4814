#ifndef TC_MODEL_CLIENT_API__H
#define TC_MODEL_CLIENT_API__H
#include <stdint.h>
#include "mesh_model.h"
#include "mesh_env.h"
#include "access_rx_process.h"
#include "generic_common.h"

typedef struct {
    /** The model base of the model. */
    model_client_base_t model;
    user_set_current_value_cb set_current_value_cb;
    user_get_current_state_cb get_current_state_cb;
}custom_tc_client_t;




#define DEF_TC_MODEL_CLIENT_MODEL(name,app_key_max) \
    static custom_tc_client_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_CUSTOM_TC_CLIENT_MODEL(name,app_key_max,element_num,sub_list,sub_list_num) \
    mesh_model_init(&name.model.base,TC_MODEL_CLIENT_MODEL_ID,false,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);



void tc_message_attr_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tc_message_attr_get_value(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);


void custom_tc_msg_publish(custom_tc_client_t *client, void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode);
#endif /*TC_MODEL_CLIENT_API__H */


