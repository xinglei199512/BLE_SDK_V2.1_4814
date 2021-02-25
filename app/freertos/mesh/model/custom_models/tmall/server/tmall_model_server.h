#ifndef TMALL_MODEL_SERVER_H_
#define TMALL_MODEL_SERVER_H_
#include <stdint.h>
#include "mesh_model.h"
#include "mesh_env.h"
#include "osapp_config.h"
#include "generic_common.h"
#include "model_servers_events_api.h"
#include "access_rx_process.h"

#define GENERCI_LEVEL_BOUND_HANDLER_MAX 5
enum {
    TMALL_MODEL_ATTR_HSL = 0x0123,
    TMALL_MODEL_ATTR_ONOFF = 0x0100,
    TMALL_MODEL_ATTR_LIGHTNESS = 0x0121,
    TMALL_MODEL_ATTR_CTL = 0x122,
    TMALL_MODEL_ATTR_MAIN_LIGHTONOFF = 0x0534,
    TMALL_MODEL_ATTR_BACKGROUNDLIGHTONOFF = 0x0533,
    TMALL_MODEL_ATTR_MODEL_NUMBER = 0xf004,
    TMALL_MODEL_ATTR_ERRER_EVENT = 0xf009
};

typedef void(*user_custom_tmall_server_cb)(access_pdu_tx_t *, uint8_t);

typedef struct
{
    uint16_t attr_type;
    uint16_t value[3];
}custom_tmall_msg_format_t;

typedef struct
{
    model_server_base_t model;
    custom_tmall_msg_format_t msg_format[1];
    generic_valid_field_queue_t tid_queue;
    mesh_model_evt_cb_t cb;
}custom_tmall_server_t;

#define DEF_TMALL_MODEL_SERVER_MODEL(name,app_key_max) \
      static custom_tmall_server_t name;\
      static model_publish_state_t name##_publish_state;\
      static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_CUSTOM_TMALL_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,TMALL_MODEL_SERVER_MODEL_ID,false,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;


void set_tmall_server(custom_tmall_server_t *server);
void tmall_msg_attr_indication_tx(void *msg, uint8_t len);

void tmall_message_transparent_msg_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tmall_message_attr_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tmall_message_attr_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tmall_attr_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tmall_message_attr_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tmall_message_attr_indication_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tmall_message_attr_confirmation_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tmall_message_attr_time_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tmall_message_attr_respond_time(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tmall_msg_attr_indication_tx(void *msg, uint8_t len);
void tmall_message_transparent_ack(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);


#endif /* TMALL_MODEL_SERVER_H_ */
