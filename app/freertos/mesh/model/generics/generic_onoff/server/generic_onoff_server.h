#ifndef GENERIC_ONOFF_SERVER_H_
#define GENERIC_ONOFF_SERVER_H_
#include <stdint.h>
#include "model_msg.h"
#include "model_common.h"
#include "generic_common.h"
#include "node_setup.h"

#define GENERIC_ONOFF_SERVER_MODEL_ID 0x1000

typedef struct
{
    uint8_t present_onoff;
    uint8_t target_onoff;
}generic_onoff_msg_format_t;


typedef struct
{
    model_server_base_t model;
    model_state_bound_field_t *state_bound;
    generic_onoff_msg_format_t msg_format;    
    generic_valid_field_queue_t tid_queue;    //messages in 6 sec , judge tid
    generic_delay_trans_param_t *delay_trans_timer;        //5ms * p_pdu->delay timer
    mesh_model_evt_cb_t cb;
    uint8_t server_state;
}generic_onoff_server_t;




#define DEF_GENERIC_ONOFF_SERVER_MODEL(name,app_key_max) \
    static generic_onoff_server_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_GENERIC_ONOFF_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,GENERIC_ONOFF_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,(model_publish_timeout_cb_t)generic_onoff_status_publish);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

void generic_onoff_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_onoff_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);
void generic_onoff_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu);

void generic_onoff_status_publish(generic_onoff_server_t *server);

#endif /* GENERIC_ONOFF_SERVER__H */


